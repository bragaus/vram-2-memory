#define _POSIX_C_SOURCE 200809L

#include "alvo_ublk.h"

#include <errno.h>
#include <limits.h>
#include <linux/ublk_cmd.h>
#include <stdint.h>
#include <time.h>

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
    if (contexto == 0 || contexto->meio == 0 ||
        quantidade_de_bytes > INT_MAX) {
        return -EINVAL;
    }
    switch (operacao) {
    case UBLK_IO_OP_READ:
        return ler_meio_simulado(contexto->meio, deslocamento, memoria,
                                 quantidade_de_bytes) ?
               (int)quantidade_de_bytes : -EIO;
    case UBLK_IO_OP_WRITE:
        return escrever_meio_simulado(contexto->meio, deslocamento, memoria,
                                      quantidade_de_bytes) ?
               (int)quantidade_de_bytes : -EIO;
    case UBLK_IO_OP_FLUSH:
        return 0;
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
        ublksrv_ctrl_stop_dev(ublksrv_get_ctrl_dev(fila_exterior->dev));
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
    uint32_t quantidade;
    uint8_t operacao;
    void *memoria;
    int resultado;

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
    if (!concluir_requisicao_na_fila(
            contexto->fila, (uint32_t)dados->tag, resultado)) return -EIO;
    resultado = ublksrv_complete_io(fila_exterior,
                                    (unsigned int)dados->tag, resultado);
    if (resultado >= 0) rearmar_requisicao_na_fila(
        contexto->fila, (uint32_t)dados->tag);
    return resultado;
}
