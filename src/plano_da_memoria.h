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

#endif
