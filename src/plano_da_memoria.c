#include "plano_da_memoria.h"

#include <errno.h>
#include <stdint.h>

/*
 * THEOREMA DO PRODUCTO FIXAVEL
 * Proposito: medir toda memória intermediária pedida pela geometria.
 * Pre-condições: destino válido e factores positivos.
 * Effeitos: publica a medida somente quando ella cabe em uint64_t.
 * Retorno: zero, -EINVAL no domínio ou -EOVERFLOW no producto.
 * Razão: multiplicar somente depois de dividir cerca o infinito arithmetico.
 */
int calcular_memoria_intermediaria(
    const struct configuracao_do_apparelho *configuracao,
    uint64_t *quantidade_em_bytes)
{
    uint64_t quantidade_de_operacoes;

    if (configuracao == 0 || quantidade_em_bytes == 0 ||
        configuracao->quantidade_de_filas <= 0 ||
        configuracao->profundidade_das_filas <= 0 ||
        configuracao->maior_operacao_em_bytes == 0) return -EINVAL;
    if ((uint64_t)configuracao->quantidade_de_filas >
        UINT64_MAX / (uint64_t)configuracao->profundidade_das_filas)
        return -EOVERFLOW;
    quantidade_de_operacoes =
        (uint64_t)configuracao->quantidade_de_filas *
        (uint64_t)configuracao->profundidade_das_filas;
    if (quantidade_de_operacoes >
        UINT64_MAX / configuracao->maior_operacao_em_bytes)
        return -EOVERFLOW;
    *quantidade_em_bytes = quantidade_de_operacoes *
        configuracao->maior_operacao_em_bytes;
    return 0;
}
