#include "servidor_ublk.h"
#include "alvo_ublk.h"
#include <errno.h>
#include <pthread.h>
#include <ublksrv.h>

/* O servidor reune a configuração, o meio e as duas faces do dispositivo. */
struct estado_do_servidor_ublk {
    const struct configuracao_do_apparelho *configuracao;
    struct meio_simulado meio;
    struct ublksrv_ctrl_dev *controle;
    const struct ublksrv_dev *dispositivo;
};
/* Cada trabalhador possue fila autoral e fila exterior de igual índice. */
struct incumbencia_da_fila_ublk {
    struct estado_do_servidor_ublk *servidor;
    struct fila_de_requisicoes fila;
    struct contexto_da_fila_ublk contexto;
    pthread_t fio_de_execucao;
    unsigned short indice;
    int resultado;
};
static struct estado_do_servidor_ublk *servidor_em_exercicio;
/*
 * LEMMA DA FIGURA DO ALVO
 * Proposito: declarar á libublksrv a capacidade e a disciplina das filas.
 * Pre-condições: servidor único preparado antes de iniciar o dispositivo.
 * Effeitos: preenche somente as grandezas autorais do alvo.
 * Retorno: zero no êxito ou erro negativo se falta contexto.
 * Razão: a bibliotheca chama sem argumento autoral; o servidor é singular.
 */
int inicializar_alvo_ublk(struct ublksrv_dev *dispositivo, int tipo,
                          int quantidade_de_argumentos, char *argumentos[])
{
    const struct ublksrv_ctrl_dev_info *informacao;

    (void)tipo;
    (void)quantidade_de_argumentos;
    (void)argumentos;
    if (dispositivo == 0 || servidor_em_exercicio == 0) return -EINVAL;
    informacao = ublksrv_ctrl_get_dev_info(
        ublksrv_get_ctrl_dev(dispositivo));
    if (informacao == 0) return -ENODEV;
    dispositivo->tgt.dev_size =
        servidor_em_exercicio->configuracao->capacidade_em_bytes;
    dispositivo->tgt.tgt_ring_depth = informacao->queue_depth;
    dispositivo->tgt.nr_fds = 0;
    dispositivo->tgt.tgt_data = servidor_em_exercicio;
    return 0;
}

/*
 * THEOREMA DO TRABALHADOR DA FILA
 * Proposito: servir uma fila exterior no seu único fio de execução.
 * Pre-condições: dispositivo e incumbência preparados antes do fio nascer.
 * Effeitos: reserva registros, processa pedidos e restitue toda a fila.
 * Retorno: o proprio argumento; o resultado fica gravado na incumbência.
 * Razão: um só proprietário dispensa synchronização dentro de cada fila.
 */
void *servir_fila_ublk(void *argumento)
{
    struct incumbencia_da_fila_ublk *incumbencia = argumento;
    const struct ublksrv_queue *fila_exterior;

    incumbencia->resultado = -ENOMEM;
    if (!criar_fila_de_requisicoes(
            &incumbencia->fila,
            incumbencia->servidor->configuracao->profundidade_das_filas)) {
        return argumento;
    }
    incumbencia->contexto.fila = &incumbencia->fila;
    incumbencia->contexto.meio = &incumbencia->servidor->meio;
    fila_exterior = ublksrv_queue_init(
        incumbencia->servidor->dispositivo, incumbencia->indice,
        &incumbencia->contexto);
    if (fila_exterior == 0) {
        destruir_fila_de_requisicoes(&incumbencia->fila);
        incumbencia->resultado = -ENODEV;
        return argumento;
    }
    do {
        incumbencia->resultado = ublksrv_process_io(fila_exterior);
    } while (incumbencia->resultado >= 0 &&
             !ublksrv_queue_is_done(fila_exterior));
    ublksrv_queue_deinit(fila_exterior);
    destruir_fila_de_requisicoes(&incumbencia->fila);
    return argumento;
}

/*
 * LEMMA DOS PARAMETROS DO BLOCO
 * Proposito: declarar capacidade, bloco e maior operação ao núcleo.
 * Pre-condições: controle vivo e configuração já julgada.
 * Effeitos: envia UBLK_CMD_SET_PARAMS. Retorno: resultado da bibliotheca.
 * Razão: sectores de 512 octetos são exactos porque o bloco mede 4096.
 */
int configurar_parametros_ublk(struct estado_do_servidor_ublk *servidor)
{
    struct ublk_params parametros = {0};

    if (servidor == 0 || servidor->controle == 0) return -EINVAL;
    parametros.len = sizeof(parametros);
    parametros.types = UBLK_PARAM_TYPE_BASIC;
    parametros.basic.attrs = UBLK_ATTR_VOLATILE_CACHE;
    parametros.basic.logical_bs_shift = 12;
    parametros.basic.physical_bs_shift = 12;
    parametros.basic.io_opt_shift = 12;
    parametros.basic.io_min_shift = 12;
    parametros.basic.max_sectors =
        servidor->configuracao->maior_operacao_em_bytes >> 9;
    parametros.basic.dev_sectors =
        servidor->configuracao->capacidade_em_bytes >> 9;
    return ublksrv_ctrl_set_params(servidor->controle, &parametros);
}

/*
 * THEOREMA DA ABERTURA DAS FILAS
 * Proposito: dar a cada fila sua incumbência e seu fio exclusivo.
 * Pre-condições: taboa dimensionada pela configuração e dispositivo vivo.
 * Effeitos: cria fios até a primeira falha e conta quantos nasceram.
 * Retorno: zero no êxito ou erro negativo conservando a contagem parcial.
 * Razão: a contagem torna possível recolher exactamente o que nasceu.
 */
int iniciar_filas_ublk(struct estado_do_servidor_ublk *servidor,
                       struct incumbencia_da_fila_ublk *incumbencias,
                       uint32_t *quantidade_iniciada)
{
    int resultado;

    if (servidor == 0 || incumbencias == 0 || quantidade_iniciada == 0) {
        return -EINVAL;
    }
    *quantidade_iniciada = 0;
    while (*quantidade_iniciada <
           (uint32_t)servidor->configuracao->quantidade_de_filas) {
        struct incumbencia_da_fila_ublk *incumbencia =
            &incumbencias[*quantidade_iniciada];
        incumbencia->servidor = servidor;
        incumbencia->indice = (unsigned short)*quantidade_iniciada;
        resultado = pthread_create(&incumbencia->fio_de_execucao, 0,
                                   servir_fila_ublk, incumbencia);
        if (resultado != 0) {
            return -resultado;
        }
        (*quantidade_iniciada)++;
    }
    return 0;
}
