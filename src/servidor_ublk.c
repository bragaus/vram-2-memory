#include "servidor_ublk.h"
#include "alvo_ublk.h"
#include "monitor_do_observatorio.h"
#include "observador_de_si.h"
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <ublksrv.h>
#include <ublksrv_utils.h>
#include <unistd.h>

/* O servidor reune a configuração, o meio e as duas faces do dispositivo. */
struct estado_do_servidor_ublk {
    const struct configuracao_do_apparelho *configuracao;
    const struct operacoes_do_meio *operacoes_do_meio;
    void *contexto_do_meio;
    struct meio_simulado meio;
    struct meio_cuda meio_cuda;
    struct ublksrv_ctrl_dev *controle;
    const struct ublksrv_dev *dispositivo;
    struct contadores_da_fila *contadores;
    pthread_t fio_do_observatorio;
    atomic_int ordenar_termo_do_observatorio;
    int observatorio_iniciado;
    int empregar_cuda;
    int memoria_fixada;
};
/* Cada trabalhador possue fila autoral e fila exterior de igual índice. */
struct incumbencia_da_fila_ublk {
    struct estado_do_servidor_ublk *servidor;
    struct fila_de_requisicoes fila;
    struct contexto_da_fila_ublk contexto;
    struct transportador_cuda transportador_cuda;
    pthread_t fio_de_execucao;
    unsigned short indice;
    int resultado;
};
static struct estado_do_servidor_ublk *servidor_em_exercicio;

/*
 * LEMMA DO GABINETE DE EXECUCAO
 * Proposito: assegurar a casa onde ublksrv conserva o processo dirigente.
 * Pre-condições: caminho absoluto, não vazio e pertencente á bibliotheca.
 * Effeitos: crea o directório ausente; não altera um directório existente.
 * Retorno: zero no êxito ou erro negativo quando o caminho não é directório.
 * Razão: a instalação local não possue gestor de serviços que prepare /run.
 */
static int preparar_directorio_de_execucao(const char *caminho)
{
    struct stat estado;

    if (caminho == 0 || caminho[0] == 0) return -EINVAL;
    if (mkdir(caminho, 0755) == 0) return 0;
    if (errno != EEXIST) return -errno;
    if (stat(caminho, &estado) != 0) return -errno;
    return S_ISDIR(estado.st_mode) ? 0 : -ENOTDIR;
}

/*
 * Proposito: descobrir novamente a largura concedida pelo terminal.
 * Pre-condições: a saída de erros poderá ou não ser um terminal.
 * Effeitos: consulta TIOCGWINSZ sem mudar o apparelho.
 * Retorno: largura observada ou oitenta columnas na ignorância.
 * Razão: cada quadro responde naturalmente a SIGWINCH na colheita seguinte.
 */
static size_t descobrir_largura_do_observatorio(void)
{
    struct winsize dimensao = {0};

    if (ioctl(STDERR_FILENO, TIOCGWINSZ, &dimensao) == 0 &&
        dimensao.ws_col != 0) return dimensao.ws_col;
    return 80;
}

/*
 * THEOREMA DO OBSERVADOR FRIO
 * Proposito: colher e mostrar retratos sem deter as filas.
 * Pre-condições: servidor preparado, contadores vivos e configuração válida.
 * Effeitos: escreve quadros periódicos até receber ordem atomica de termo.
 * Retorno: o argumento original; perdas de saída tornam-se estatística.
 * Razão: espera e apresentação vivem fora do caminho das requisições.
 */
static void *observar_servidor_ublk(void *argumento)
{
    struct estado_do_servidor_ublk *servidor = argumento;
    struct retrato_do_observatorio retrato, anterior = {0}, janella;
    struct configuracao_do_monitor configuracao;
    struct configuracao_da_narracao configuracao_da_voz;
    const struct timespec repouso = {1, 0};
    uint64_t instante_anterior = ler_instante_monotonico();
    char quadro[2048];
    char voz[256];

    while (!atomic_load_explicit(&servidor->ordenar_termo_do_observatorio,
                                 memory_order_relaxed)) {
        uint64_t instante_actual = ler_instante_monotonico();
        size_t tamanho;
        ssize_t escriptos;

        anterior.instante_monotonico_em_nanossegundos = instante_anterior;
        if (!colher_retrato_do_observatorio(
                &retrato, servidor->contadores,
                (size_t)servidor->configuracao->quantidade_de_filas,
                instante_actual, instante_anterior)) break;
        retrato.capacidade_em_bytes =
            servidor->configuracao->capacidade_em_bytes;
        retrato.memoria_do_meio_reservada_em_bytes =
            retrato.capacidade_em_bytes;
        retrato.memoria_da_cpu_fixada_em_bytes =
            (uint64_t)servidor->configuracao->quantidade_de_filas *
            servidor->configuracao->profundidade_das_filas *
            servidor->configuracao->maior_operacao_em_bytes;
        if (!differenciar_retratos_do_observatorio(
                &janella, &retrato, &anterior)) break;
        configuracao.largura_em_colunas = descobrir_largura_do_observatorio();
        configuracao.empregar_cor = isatty(STDERR_FILENO);
        tamanho = escrever_quadro_do_observatorio(
            quadro, sizeof(quadro), &janella, &configuracao);
        escriptos = tamanho == 0 ? -1 :
            write(STDERR_FILENO, quadro, tamanho);
        if (escriptos < 0 || (size_t)escriptos != tamanho)
            atomic_fetch_add_explicit(&servidor->contadores[0].amostras_perdidas,
                                      1, memory_order_relaxed);
        configuracao_da_voz.modo = configuracao.empregar_cor ?
            MODO_DA_NARRACAO_THEATRAL : MODO_DA_NARRACAO_SOBRIO;
        configuracao_da_voz.idade_maxima_em_nanossegundos = 2000000000ULL;
        configuracao_da_voz.p99_alarmante_em_microssegundos =
            (uint64_t)servidor->configuracao
                ->prazo_da_operacao_em_milissegundos * 1000ULL;
        tamanho = narrar_observador_de_si(
            voz, sizeof(voz), &janella, &configuracao_da_voz, instante_actual);
        escriptos = tamanho == SIZE_MAX ? -1 :
            write(STDERR_FILENO, voz, tamanho);
        if (escriptos < 0 || (size_t)escriptos != tamanho)
            atomic_fetch_add_explicit(&servidor->contadores[0].amostras_perdidas,
                                      1, memory_order_relaxed);
        anterior = retrato;
        instante_anterior = instante_actual;
        nanosleep(&repouso, 0);
    }
    return argumento;
}

/*
 * Proposito: fazer nascer o observador somente depois das filas preparadas.
 * Pre-condições: servidor e taboa estatística permanecem vivos.
 * Effeitos: inicia um fio frio e registra sua existência.
 * Retorno: zero no êxito ou erro negativo de pthread_create.
 * Razão: a marca só se publica depois que o fio realmente nasceu.
 */
static int iniciar_observatorio_ublk(struct estado_do_servidor_ublk *servidor)
{
    int resultado;

    if (servidor == 0 || servidor->contadores == 0) return -EINVAL;
    atomic_store_explicit(&servidor->ordenar_termo_do_observatorio, 0,
                          memory_order_relaxed);
    resultado = pthread_create(&servidor->fio_do_observatorio, 0,
                               observar_servidor_ublk, servidor);
    if (resultado != 0) return -resultado;
    servidor->observatorio_iniciado = 1;
    return 0;
}

/*
 * Proposito: ordenar o termo e recolher exactamente o observador nascido.
 * Pre-condições: servidor válido; a marca declara a posse do fio.
 * Effeitos: publica a ordem, reúne o fio e apaga a marca.
 * Retorno: zero ou erro negativo de pthread_join.
 * Razão: contadores só podem morrer depois que cessou quem os contempla.
 */
static int encerrar_observatorio_ublk(struct estado_do_servidor_ublk *servidor)
{
    int resultado;

    if (servidor == 0) return -EINVAL;
    if (!servidor->observatorio_iniciado) return 0;
    atomic_store_explicit(&servidor->ordenar_termo_do_observatorio, 1,
                          memory_order_relaxed);
    resultado = pthread_join(servidor->fio_do_observatorio, 0);
    servidor->observatorio_iniciado = 0;
    return resultado == 0 ? 0 : -resultado;
}
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
    incumbencia->contexto.contadores =
        &incumbencia->servidor->contadores[incumbencia->indice];
    incumbencia->contexto.operacoes_do_meio =
        incumbencia->servidor->operacoes_do_meio;
    incumbencia->contexto.contexto_do_meio =
        incumbencia->servidor->contexto_do_meio;
    incumbencia->contexto.indice_da_fila = incumbencia->indice;
    incumbencia->resultado = incumbencia->contexto.operacoes_do_meio
        ->vincular_fila(incumbencia->contexto.contexto_do_meio,
                       incumbencia->contexto.indice_da_fila);
    if (incumbencia->resultado < 0) {
        destruir_fila_de_requisicoes(&incumbencia->fila);
        return argumento;
    }
    if (incumbencia->servidor->empregar_cuda) {
        incumbencia->contexto.transportador_cuda =
            &incumbencia->transportador_cuda;
    } else {
        incumbencia->contexto.meio_simulado = &incumbencia->servidor->meio;
    }
    incumbencia->contexto.prazo_em_nanossegundos =
        (uint64_t)incumbencia->servidor->configuracao
            ->prazo_da_operacao_em_milissegundos * 1000000ULL;
    fila_exterior = ublksrv_queue_init(
        incumbencia->servidor->dispositivo, incumbencia->indice,
        &incumbencia->contexto);
    if (fila_exterior == 0) {
        destruir_transportador_cuda(&incumbencia->transportador_cuda);
        destruir_fila_de_requisicoes(&incumbencia->fila);
        incumbencia->resultado = -ENODEV;
        return argumento;
    }
    do {
        incumbencia->resultado = ublksrv_process_io(fila_exterior);
    } while (incumbencia->resultado >= 0 &&
             !ublksrv_queue_is_done(fila_exterior));
    /* A parada ordenada poderá romper a espera com código exterior negativo. */
    if (ublksrv_queue_is_done(fila_exterior)) {
        incumbencia->resultado = 0;
    } else if (incumbencia->resultado < 0) {
        /* Uma fila enferma ordena que suas irmãs também convirjam. */
        ublksrv_ctrl_stop_dev(incumbencia->servidor->controle);
    }
    ublksrv_queue_deinit(fila_exterior);
    if (!destruir_transportador_cuda(&incumbencia->transportador_cuda) &&
        incumbencia->resultado == 0) {
        incumbencia->resultado = -EIO;
    }
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
    parametros.types = UBLK_PARAM_TYPE_BASIC | UBLK_PARAM_TYPE_DISCARD;
    parametros.basic.attrs = UBLK_ATTR_VOLATILE_CACHE | UBLK_ATTR_FUA;
    parametros.basic.logical_bs_shift = 12;
    parametros.basic.physical_bs_shift = 12;
    parametros.basic.io_opt_shift = 12;
    parametros.basic.io_min_shift = 12;
    parametros.basic.max_sectors =
        servidor->configuracao->maior_operacao_em_bytes >> 9;
    parametros.basic.dev_sectors =
        servidor->configuracao->capacidade_em_bytes >> 9;
    parametros.discard.discard_alignment = TAMANHO_DO_BLOCO_EM_BYTES;
    parametros.discard.discard_granularity = TAMANHO_DO_BLOCO_EM_BYTES;
    parametros.discard.max_discard_sectors =
        servidor->configuracao->maior_operacao_em_bytes >> 9;
    parametros.discard.max_write_zeroes_sectors =
        servidor->configuracao->maior_operacao_em_bytes >> 9;
    parametros.discard.max_discard_segments = 1;
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
        if (servidor->empregar_cuda &&
            !criar_transportador_cuda(&incumbencia->transportador_cuda,
                                      &servidor->meio_cuda)) {
            return -EIO;
        }
        resultado = pthread_create(&incumbencia->fio_de_execucao, 0,
                                   servir_fila_ublk, incumbencia);
        if (resultado != 0) {
            destruir_transportador_cuda(&incumbencia->transportador_cuda);
            return -resultado;
        }
        (*quantidade_iniciada)++;
    }
    return 0;
}

/*
 * COROLLARIO DA RECOLHA DAS FILAS
 * Proposito: reunir todos os fios nascidos e conservar a primeira falha.
 * Pre-condições: a quantidade é aquella contada durante a abertura.
 * Effeitos: espera o termo de cada fio. Retorno: zero ou primeiro erro.
 * Razão: nenhuma falha posterior deve occultar a causa mais antiga.
 */
int recolher_filas_ublk(struct incumbencia_da_fila_ublk *incumbencias,
                        uint32_t quantidade)
{
    uint32_t indice;
    int primeiro_resultado = 0;

    if (incumbencias == 0 && quantidade != 0) return -EINVAL;
    for (indice = 0; indice < quantidade; indice++) {
        int resultado = pthread_join(
            incumbencias[indice].fio_de_execucao, 0);
        if (primeiro_resultado == 0 && resultado != 0) {
            primeiro_resultado = -resultado;
        }
        if (primeiro_resultado == 0 &&
            incumbencias[indice].resultado < 0) {
            primeiro_resultado = incumbencias[indice].resultado;
        }
    }
    return primeiro_resultado;
}

/*
 * LEMMA DAS OPERACOES DO ALVO
 * Proposito: reunir as duas chamadas obrigatórias de libublksrv.
 * Pre-condições: os contractos de inicialização e passagem permanecem vivos.
 * Effeitos: nenhum. Retorno: endereço immutável da taboa singular.
 * Razão: uma taboa estática jámais depende do tempo de vida da pilha.
 */
const struct ublksrv_tgt_type *obter_operacoes_do_alvo_ublk(void)
{
    static const struct ublksrv_tgt_type operacoes = {
        .name = "vram_2_memory",
        .init_tgt = inicializar_alvo_ublk,
        .handle_io_async = tratar_requisicao_ublk
    };

    return &operacoes;
}

/*
 * COROLLARIO DAS OPERACOES CUDA
 * Proposito: acrescentar buffers fixados á passagem commum das requisições.
 * Pre-condições: execução CUDA disponível antes de iniciar as filas.
 * Effeitos: nenhum. Retorno: endereço immutável da taboa CUDA.
 * Razão: somente esta variante promette DMA e por isso fixa seus buffers.
 */
const struct ublksrv_tgt_type *obter_operacoes_do_alvo_cuda(void)
{
    static const struct ublksrv_tgt_type operacoes = {
        .name = "vram_2_memory_cuda",
        .init_tgt = inicializar_alvo_ublk,
        .handle_io_async = tratar_requisicao_ublk,
        .alloc_io_buf = reservar_memoria_ublk_cuda,
        .free_io_buf = destruir_memoria_ublk_cuda
    };

    return &operacoes;
}

/*
 * COROLLARIO DA ORDEM DE PARADA
 * Proposito: converter a interrupção exterior em termo do dispositivo.
 * Pre-condições: o servidor singular já publicou seu controle.
 * Effeitos: solicita parada á libublksrv. Retorno: nenhum.
 * Razão: o sinal só aponta a porta pela qual os fios hão de convergir.
 */
void ordenar_parada_do_servidor_ublk(int sinal_recebido)
{
    (void)sinal_recebido;
    if (servidor_em_exercicio != 0 &&
        servidor_em_exercicio->controle != 0) {
        ublksrv_ctrl_stop_dev(servidor_em_exercicio->controle);
    }
}

/*
 * LEMMA DA PORTA DE CONTROLE
 * Proposito: crear no núcleo o par de dispositivos ainda não publicado.
 * Pre-condições: configuração cabe nas larguras impostas por ublk.
 * Effeitos: adquire controle e acrescenta dispositivo.
 * Retorno: zero no êxito ou erro negativo com limpeza na falha.
 * Razão: a porta deve existir antes das filas, mas não recebe tráfego ainda.
 */
int abrir_controle_ublk(struct estado_do_servidor_ublk *servidor)
{
    struct ublksrv_dev_data dados = {0};
    int resultado;

    if (servidor == 0 || servidor->configuracao == 0 ||
        servidor->configuracao->quantidade_de_filas > USHRT_MAX ||
        (unsigned int)servidor->configuracao->quantidade_de_filas >
            UBLK_MAX_NR_QUEUES ||
        servidor->configuracao->profundidade_das_filas > UBLK_MAX_QUEUE_DEPTH ||
        servidor->configuracao->maior_operacao_em_bytes > INT_MAX) {
        return -EINVAL;
    }
    dados.dev_id = -1;
    dados.max_io_buf_bytes =
        servidor->configuracao->maior_operacao_em_bytes;
    dados.nr_hw_queues =
        (unsigned short)servidor->configuracao->quantidade_de_filas;
    dados.queue_depth =
        (unsigned short)servidor->configuracao->profundidade_das_filas;
    dados.tgt_type = servidor->empregar_cuda ?
        "vram_2_memory_cuda" : "vram_2_memory";
    dados.tgt_ops = servidor->empregar_cuda ?
        obter_operacoes_do_alvo_cuda() : obter_operacoes_do_alvo_ublk();
    dados.run_dir = ublksrv_get_pid_dir();
    resultado = preparar_directorio_de_execucao(dados.run_dir);
    if (resultado < 0) return resultado;
    servidor->controle = ublksrv_ctrl_init(&dados);
    if (servidor->controle == 0) return -ENODEV;
    resultado = ublksrv_ctrl_add_dev(servidor->controle);
    if (resultado < 0) {
        ublksrv_ctrl_deinit(servidor->controle);
        servidor->controle = 0;
    }
    return resultado;
}

/*
 * COROLLARIO DO DESMONTE DO SERVIDOR
 * Proposito: restituir dispositivo, controle e meio na ordem inversa.
 * Pre-condições: todos os fios já convergiram ou nenhum foi iniciado.
 * Effeitos: remove o dispositivo do núcleo e liberta toda posse local.
 * Retorno: resultado da remoção, sem omittir a limpeza na falha.
 * Razão: cada camada exterior morre antes da matéria que ella referencia.
 */
int desmontar_servidor_ublk(struct estado_do_servidor_ublk *servidor)
{
    int resultado = 0;

    if (servidor == 0) return -EINVAL;
    if (servidor->dispositivo != 0) {
        ublksrv_dev_deinit(servidor->dispositivo);
        servidor->dispositivo = 0;
    }
    if (servidor->controle != 0) {
        resultado = ublksrv_ctrl_del_dev(servidor->controle);
        ublksrv_ctrl_deinit(servidor->controle);
        servidor->controle = 0;
    }
    if (servidor->operacoes_do_meio != 0 &&
        servidor->contexto_do_meio != 0) {
        servidor->operacoes_do_meio->destruir(servidor->contexto_do_meio);
        servidor->contexto_do_meio = 0;
    }
    destruir_meio_simulado(&servidor->meio);
    if (!destruir_meio_cuda(&servidor->meio_cuda) && resultado == 0) {
        resultado = -EIO;
    }
    if (servidor->memoria_fixada) {
        if (munlockall() != 0 && resultado == 0) resultado = -errno;
        servidor->memoria_fixada = 0;
    }
    if (servidor_em_exercicio == servidor) {
        servidor_em_exercicio = 0;
    }
    return resultado;
}

/*
 * THEOREMA DA PREPARACAO INTEGRAL
 * Proposito: adquirir meio, controle, dispositivo e parâmetros antes das filas.
 * Pre-condições: configuração válida e nenhum outro servidor em exercício.
 * Effeitos: estabelece todas as camadas que antecedem os trabalhadores.
 * Retorno: zero no êxito ou erro negativo depois de restituição completa.
 * Razão: a publicação só virá quando toda memória e geometria existirem.
 */
int preparar_servidor_ublk(struct estado_do_servidor_ublk *servidor,
                           const struct configuracao_do_apparelho *configuracao,
                           int empregar_cuda)
{
    int resultado;

    if (servidor == 0 || !configuracao_do_apparelho_e_valida(configuracao) ||
        servidor_em_exercicio != 0) return -EINVAL;
    servidor->configuracao = configuracao;
    servidor->empregar_cuda = empregar_cuda != 0;
    servidor->operacoes_do_meio = servidor->empregar_cuda ?
        obter_operacoes_do_meio_cuda() : obter_operacoes_do_meio_simulado();
    /* O servidor de swap jámais poderá depender do proprio dispositivo. */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) return -errno;
    servidor->memoria_fixada = 1;
    resultado = servidor->operacoes_do_meio->preparar(
        &servidor->contexto_do_meio, configuracao);
    if (resultado < 0) {
        desmontar_servidor_ublk(servidor);
        return resultado;
    }
    servidor_em_exercicio = servidor;
    resultado = abrir_controle_ublk(servidor);
    if (resultado < 0) {
        desmontar_servidor_ublk(servidor);
        return resultado;
    }
    servidor->dispositivo = ublksrv_dev_init(servidor->controle);
    if (servidor->dispositivo == 0) {
        desmontar_servidor_ublk(servidor);
        return -ENODEV;
    }
    resultado = configurar_parametros_ublk(servidor);
    if (resultado == 0) {
        /* A affinidade precede o nascimento dos fios que ella orientará. */
        resultado = ublksrv_ctrl_get_affinity(servidor->controle);
    }
    if (resultado < 0) desmontar_servidor_ublk(servidor);
    return resultado;
}

/*
 * THEOREMA DO SERVICO UBLK
 * Proposito: preparar, publicar, servir e desmontar o apparelho completo.
 * Pre-condições: configuração julgada e libublksrv disponível.
 * Effeitos: expõe bloco volátil até interrupção ou falha de fila.
 * Retorno: zero no termo regular ou a primeira falha negativa.
 * Razão: toda saída converge pela mesma successão inversa de limpeza.
 */
int executar_servidor_com_meio(
    const struct configuracao_do_apparelho *configuracao, int empregar_cuda)
{
    struct estado_do_servidor_ublk servidor = {0};
    struct incumbencia_da_fila_ublk *incumbencias;
    uint32_t quantidade_iniciada = 0;
    int resultado;
    int resultado_do_desmonte;

    resultado = preparar_servidor_ublk(
        &servidor, configuracao, empregar_cuda);
    if (resultado < 0) return resultado;
    incumbencias = calloc(
        (size_t)configuracao->quantidade_de_filas, sizeof(*incumbencias));
    servidor.contadores = calloc(
        (size_t)configuracao->quantidade_de_filas, sizeof(*servidor.contadores));
    if (incumbencias == 0 || servidor.contadores == 0) {
        free(servidor.contadores);
        free(incumbencias);
        desmontar_servidor_ublk(&servidor);
        return -ENOMEM;
    }
    if (signal(SIGINT, ordenar_parada_do_servidor_ublk) == SIG_ERR ||
        signal(SIGTERM, ordenar_parada_do_servidor_ublk) == SIG_ERR) {
        free(servidor.contadores);
        free(incumbencias);
        desmontar_servidor_ublk(&servidor);
        return -errno;
    }
    resultado = iniciar_filas_ublk(
        &servidor, incumbencias, &quantidade_iniciada);
    if (resultado == 0) {
        resultado = ublksrv_ctrl_start_dev(servidor.controle, getpid());
    }
    if (resultado == 0) resultado = iniciar_observatorio_ublk(&servidor);
    if (resultado < 0) ublksrv_ctrl_stop_dev(servidor.controle);
    {
        int resultado_das_filas = recolher_filas_ublk(
            incumbencias, quantidade_iniciada);
        if (resultado == 0) resultado = resultado_das_filas;
    }
    {
        int resultado_do_observatorio = encerrar_observatorio_ublk(&servidor);
        if (resultado == 0) resultado = resultado_do_observatorio;
    }
    free(incumbencias);
    free(servidor.contadores);
    resultado_do_desmonte = desmontar_servidor_ublk(&servidor);
    if (resultado == 0) resultado = resultado_do_desmonte;
    return resultado;
}

/*
 * Proposito: servir a experiência ublk sobre RAM ordinária.
 * Pre-condições: configuração válida. Effeitos: governa o serviço completo.
 * Retorno: zero ou primeira falha. Razão: conserva a prova sem GPU.
 */
int executar_servidor_ublk(
    const struct configuracao_do_apparelho *configuracao)
{
    return executar_servidor_com_meio(configuracao, 0);
}

/*
 * Proposito: servir o apparelho ublk sobre VRAM e DMA CUDA.
 * Pre-condições: configuração, GPU e dependências exteriores válidas.
 * Effeitos: governa o serviço completo. Retorno: zero ou primeira falha.
 * Razão: esta entrada activa correntes e buffers fixados em cada fila.
 */
int executar_servidor_cuda(
    const struct configuracao_do_apparelho *configuracao)
{
    return executar_servidor_com_meio(configuracao, 1);
}
