#define _POSIX_C_SOURCE 200809L

#include "alvo_ublk.h"

#include <errno.h>
#include <limits.h>
#include <linux/ublk_cmd.h>
#include <stdint.h>
#include <time.h>

/* Conserva a sentença colhida durante a passagem ainda synchrona do alvo. */
struct resultado_da_transferencia {
    int foi_concluida;
    int erro;
};

/*
 * Proposito: receber a sentença commum e torná-la resultado immediato.
 * Pre-condições: argumento aponta para testemunho vivo na pilha chamadora.
 * Effeitos: grava erro e marca conclusão. Retorno: nenhum.
 * Razão: a transição conserva a API antiga até o alvo tornar-se assíncrono.
 */
static void concluir_transferencia_do_meio(void *argumento, int erro)
{
    struct resultado_da_transferencia *resultado = argumento;

    resultado->erro = erro;
    resultado->foi_concluida = 1;
}

/*
 * LEMMA DO INSTANTE MONOTONICO
 * Proposito: medir a marcha da requisição sem sujeição ao calendário civil.
 * Pre-condições: o systema fornece CLOCK_MONOTONIC.
 * Effeitos: consulta o relógio. Retorno: nanossegundos, ou zero na falha.
 * Razão: segundos são cercados antes do producto que os converte.
 */
uint64_t ler_instante_monotonico(void)
{
    struct timespec instante;

    if (clock_gettime(CLOCK_MONOTONIC, &instante) != 0 ||
        (uint64_t)instante.tv_sec > UINT64_MAX / 1000000000ULL) {
        return 0;
    }
    return (uint64_t)instante.tv_sec * 1000000000ULL +
           (uint64_t)instante.tv_nsec;
}

/*
 * LEMMA DA RESERVA EXTERIOR FIXA
 * Proposito: adaptar a acquisição CUDA á assinatura de libublksrv.
 * Pre-condições: tamanho positivo. Effeitos: reserva memória CPU fixada.
 * Retorno: endereço ou nulo. Razão: fila e etiqueta não mudam a grandeza.
 */
void *reservar_memoria_ublk_cuda(const struct ublksrv_queue *fila_exterior,
                                 int etiqueta, int tamanho)
{
    (void)fila_exterior;
    (void)etiqueta;
    if (tamanho <= 0) return 0;
    return reservar_memoria_intermediaria_cuda((uint32_t)tamanho);
}

/*
 * COROLLARIO DA RESTITUICAO EXTERIOR
 * Proposito: adaptar cudaFreeHost á assinatura sem retorno da bibliotheca.
 * Pre-condições: memória fixada ou nula. Effeitos: restitue a região.
 * Retorno: nenhum. Razão: fila e etiqueta não participam da posse material.
 */
void destruir_memoria_ublk_cuda(const struct ublksrv_queue *fila_exterior,
                                void *memoria, int etiqueta)
{
    (void)fila_exterior;
    (void)etiqueta;
    destruir_memoria_intermediaria_cuda(memoria);
}
/*
 * THEOREMA DA PASSAGEM PELO CONTRACTO
 * Proposito: submetter, colher e traduzir uma operação do meio commum.
 * Pre-condições: contexto íntegro e quantidade representável em int.
 * Effeitos: transporta ou zera e colhe uma única sentença da mesma fila.
 * Retorno: octetos, zero na descarga ou erro negativo.
 * Razão: a espera transitória conserva a conducta até o alvo assíncrono.
 */
static int transferir_pelo_contrato_do_meio(
    struct contexto_da_fila_ublk *contexto, uint8_t operacao,
    uint64_t deslocamento, void *memoria, uint32_t quantidade_de_bytes)
{
    struct resultado_da_transferencia resultado = {0};
    const struct operacoes_do_meio *operacoes = contexto->operacoes_do_meio;
    int submissao;
    if (operacoes == 0 || contexto->contexto_do_meio == 0 ||
        quantidade_de_bytes > INT_MAX) return -EINVAL;
    switch (operacao) {
    case UBLK_IO_OP_READ:
        submissao = operacoes->ler(
            contexto->contexto_do_meio, contexto->indice_da_fila,
            deslocamento, memoria, quantidade_de_bytes,
            concluir_transferencia_do_meio, &resultado);
        break;
    case UBLK_IO_OP_WRITE:
        submissao = operacoes->escrever(
            contexto->contexto_do_meio, contexto->indice_da_fila,
            deslocamento, memoria, quantidade_de_bytes,
            concluir_transferencia_do_meio, &resultado);
        break;
    case UBLK_IO_OP_DISCARD:
    case UBLK_IO_OP_WRITE_ZEROES:
        submissao = operacoes->zerar(
            contexto->contexto_do_meio, contexto->indice_da_fila,
            deslocamento, quantidade_de_bytes,
            concluir_transferencia_do_meio, &resultado);
        break;
    case UBLK_IO_OP_FLUSH:
        return 0;
    default:
        return -EOPNOTSUPP;
    }
    if (submissao < 0) return submissao;
    if (operacoes->colher(contexto->contexto_do_meio,
                          contexto->indice_da_fila, 1) != 1 ||
        !resultado.foi_concluida) return -EIO;
    return resultado.erro == 0 ? (int)quantidade_de_bytes : resultado.erro;
}

/*
 * THEOREMA DA TRANSFERENCIA UBLK
 * Proposito: traduzir a operação exterior para o meio da Casa.
 * Pre-condições: contexto e meio vivos; região arithmeticamente cercada.
 * Effeitos: lê, escreve ou reconhece descarga já síncrona.
 * Retorno: medida transportada, zero na descarga ou erro negativo.
 * Razão: somente leitura, escripta e descarga pertencem a este apparelho.
 */
int transferir_requisicao_ublk(struct contexto_da_fila_ublk *contexto,
                               uint8_t operacao, uint64_t deslocamento,
                               void *memoria, uint32_t quantidade_de_bytes)
{
    if (contexto == 0 || quantidade_de_bytes > INT_MAX) return -EINVAL;
    if (contexto->operacoes_do_meio != 0 ||
        contexto->contexto_do_meio != 0) {
        if (contexto->operacoes_do_meio == 0 ||
            contexto->contexto_do_meio == 0) return -EINVAL;
        return transferir_pelo_contrato_do_meio(
            contexto, operacao, deslocamento, memoria, quantidade_de_bytes);
    }
    if (contexto->transportador_cuda == 0) return -EINVAL;
    switch (operacao) {
    case UBLK_IO_OP_READ:
        return ler_meio_cuda(contexto->transportador_cuda, deslocamento,
                             memoria, quantidade_de_bytes) ?
               (int)quantidade_de_bytes : -EIO;
    case UBLK_IO_OP_WRITE:
        return escrever_meio_cuda(contexto->transportador_cuda,
                                  deslocamento, memoria,
                                  quantidade_de_bytes) ?
               (int)quantidade_de_bytes : -EIO;
    case UBLK_IO_OP_FLUSH:
        return 0;
    case UBLK_IO_OP_DISCARD:
    case UBLK_IO_OP_WRITE_ZEROES:
        return zerar_meio_cuda(contexto->transportador_cuda, deslocamento,
                               quantidade_de_bytes) ?
               (int)quantidade_de_bytes : -EIO;
    default:
        return -EOPNOTSUPP;
    }
}

/*
 * THEOREMA DA ENTREGA TEMPESTIVA
 * Proposito: concluir dentro do prazo ou fallir e parar o apparelho.
 * Pre-condições: etiqueta transferindo e instante final monotónico.
 * Effeitos: entrega ao núcleo; rearma no êxito ou torna a falha terminal.
 * Retorno: resultado exterior ou -ETIMEDOUT quando o prazo se consome.
 * Razão: uma fila vencida não poderá receber trabalho novo honestamente.
 */
int entregar_requisicao_ublk(struct contexto_da_fila_ublk *contexto,
                             const struct ublksrv_queue *fila_exterior,
                             uint32_t etiqueta, int resultado,
                             uint64_t instante_final)
{
    int resultado_da_entrega;

    if (contexto == 0 || fila_exterior == 0) return -EINVAL;
    if (falhar_requisicao_vencida(
            contexto->fila, etiqueta, instante_final,
            contexto->prazo_em_nanossegundos, -ETIMEDOUT)) {
        ublksrv_complete_io(fila_exterior, etiqueta, -ETIMEDOUT);
        ublksrv_ctrl_stop_dev((struct ublksrv_ctrl_dev *)
                              ublksrv_get_ctrl_dev(fila_exterior->dev));
        return -ETIMEDOUT;
    }
    if (!concluir_requisicao_na_fila(
            contexto->fila, etiqueta, resultado)) return -EIO;
    resultado_da_entrega = ublksrv_complete_io(
        fila_exterior, etiqueta, resultado);
    if (resultado_da_entrega >= 0 &&
        !rearmar_requisicao_na_fila(contexto->fila, etiqueta)) return -EIO;
    return resultado_da_entrega;
}

/*
 * THEOREMA DA PASSAGEM UBLK
 * Proposito: possuir, transportar e entregar uma requisição exterior.
 * Pre-condições: fila e dados pertencem ao mesmo contexto publicado.
 * Effeitos: conclue a etiqueta uma vez e só então a restitue.
 * Retorno: zero na entrega ou erro negativo antes de nova busca.
 * Razão: sectores são cercados antes da conversão para octetos.
 */
int tratar_requisicao_ublk(const struct ublksrv_queue *fila_exterior,
                           const struct ublk_io_data *dados)
{
    struct contexto_da_fila_ublk *contexto;
    const struct ublksrv_io_desc *descritor;
    struct registro_da_requisicao *registro;
    uint64_t deslocamento;
    uint64_t instante_final;
    uint64_t latencia;
    uint32_t quantidade;
    uint8_t operacao;
    void *memoria;
    int resultado;
    int resultado_da_entrega;

    if (fila_exterior == 0 || dados == 0 || dados->iod == 0) return -EINVAL;
    contexto = fila_exterior->private_data;
    descritor = dados->iod;
    if (contexto == 0 || contexto->fila == 0 || dados->tag < 0 ||
        descritor->nr_sectors > UINT32_MAX / 512U ||
        descritor->start_sector > UINT64_MAX / 512U) return -EINVAL;
    quantidade = descritor->nr_sectors * 512U;
    deslocamento = descritor->start_sector * 512ULL;
    operacao = ublksrv_get_op(descritor);
    memoria = ublksrv_queue_get_io_buf(fila_exterior, dados->tag);
    if (quantidade != 0 && memoria == 0) return -EFAULT;
    registro = iniciar_requisicao_na_fila(
        contexto->fila, (uint32_t)dados->tag, deslocamento, quantidade,
        operacao, memoria, ler_instante_monotonico());
    if (registro == 0) return -EBUSY;
    resultado = transferir_requisicao_ublk(
        contexto, operacao, deslocamento, memoria, quantidade);
    instante_final = ler_instante_monotonico();
    latencia = instante_final >= registro->instante_inicial_em_nanossegundos ?
        instante_final - registro->instante_inicial_em_nanossegundos : 0;
    resultado_da_entrega = entregar_requisicao_ublk(
        contexto, fila_exterior, (uint32_t)dados->tag, resultado,
        instante_final);
    registrar_operacao_observada(
        contexto->contadores, operacao != UBLK_IO_OP_READ, quantidade,
        latencia, resultado_da_entrega < 0 ? resultado_da_entrega : resultado);
    return resultado_da_entrega;
}
