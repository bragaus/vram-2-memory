#include "meio_simulado.h"

#include <stdint.h>
#include <stdlib.h>

/*
 * LEMMA DO INTERVALLO CONTIDO
 * Proposito: julgar uma região sem sommar grandezas antes do limite.
 * Pre-condições: nenhuma; todo registro estranho será recusado.
 * Effeitos: nenhum. Retorno: unidade somente para região não vazia contida.
 * Razão: subtrahir depois de ordenar impede transbordamento arithmetico.
 */
static int intervallo_do_meio_e_valido(const struct meio_simulado *meio,
                                       uint64_t deslocamento,
                                       uint32_t quantidade_de_bytes)
{
    if (meio == 0 || meio->memoria == 0 || quantidade_de_bytes == 0 ||
        deslocamento > meio->capacidade_em_bytes) {
        return 0;
    }
    return quantidade_de_bytes <= meio->capacidade_em_bytes - deslocamento;
}

/*
 * THEOREMA DA RESERVA ANTECIPADA
 * Proposito: adquirir e zerar toda memória do meio antes de o publicar.
 * Pre-condições: registro não nulo e vazio; capacidade positiva e alojável.
 * Effeitos: estabelece memória e capacidade somente quando logra êxito.
 * Retorno: unidade no êxito e zero sem alterar o registro na falha.
 * Razão: a reserva única aparta o caminho crítico da pressão ulterior.
 */
int criar_meio_simulado(struct meio_simulado *meio,
                        uint64_t capacidade_em_bytes)
{
    unsigned char *memoria;

    if (meio == 0 || meio->memoria != 0 || capacidade_em_bytes == 0 ||
        capacidade_em_bytes > SIZE_MAX) {
        return 0;
    }
    memoria = calloc((size_t)capacidade_em_bytes, 1);
    if (memoria == 0) {
        return 0;
    }
    meio->memoria = memoria;
    meio->capacidade_em_bytes = capacidade_em_bytes;
    return 1;
}
