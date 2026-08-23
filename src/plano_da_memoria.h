#ifndef PLANO_DA_MEMORIA_H
#define PLANO_DA_MEMORIA_H

#include "configuracao.h"

#include <stdint.h>

/*
 * Proposito: calcular filas vezes profundidade vezes maior operação.
 * Pre-condições: destino válido e três factores positivos.
 * Effeitos: escreve a quantidade somente no êxito. Retorno: zero ou -errno.
 * Razão: nenhum producto poderá transbordar antes de reclamar RAM fixada.
 */
int calcular_memoria_intermediaria(
    const struct configuracao_do_apparelho *configuracao,
    uint64_t *quantidade_em_bytes);

/*
 * Proposito: confrontar a memória necessária com o limite fixável observado.
 * Pre-condições: destino válido e geometria calculável.
 * Effeitos: publica a necessidade mesmo quando excede. Retorno: zero ou erro.
 * Razão: a recusa deverá dizer quanto falta, não somente negar a reserva.
 */
int conferir_limite_da_memoria_intermediaria(
    const struct configuracao_do_apparelho *configuracao,
    uint64_t limite_em_bytes, uint64_t *necessaria_em_bytes);

#endif
