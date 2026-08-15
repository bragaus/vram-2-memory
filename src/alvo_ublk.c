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
