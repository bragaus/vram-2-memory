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

/*
 * COROLLARIO DO LIMITE FIXAVEL
 * Proposito: confrontar a reserva calculada com a concessão do processo.
 * Pre-condições: destino válido e geometria calculável.
 * Effeitos: publica a necessidade ainda que ella seja recusada.
 * Retorno: zero, erro do cálculo ou -ENOMEM quando o limite não basta.
 * Razão: o diagnóstico conserva ambas as grandezas para orientar correcção.
 */
int conferir_limite_da_memoria_intermediaria(
    const struct configuracao_do_apparelho *configuracao,
    uint64_t limite_em_bytes, uint64_t *necessaria_em_bytes)
{
    uint64_t necessidade;
    int resultado;

    if (necessaria_em_bytes == 0) return -EINVAL;
    resultado = calcular_memoria_intermediaria(configuracao, &necessidade);
    if (resultado < 0) return resultado;
    *necessaria_em_bytes = necessidade;
    return necessidade <= limite_em_bytes ? 0 : -ENOMEM;
}
