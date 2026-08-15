#include "meio_simulado.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

/*
 * COROLLARIO DA RESTITUICAO
 * Proposito: devolver a reserva e reduzir o registro á figura vazia.
 * Pre-condições: nenhuma; o ponteiro nulo é operação sem effeito.
 * Effeitos: liberta a memória possuída e apaga suas grandezas.
 * Retorno: nenhum. Razão: o zero impede uso e libertação repetidos.
 */
void destruir_meio_simulado(struct meio_simulado *meio)
{
    if (meio == 0) {
        return;
    }
    free(meio->memoria);
    meio->memoria = 0;
    meio->capacidade_em_bytes = 0;
}

/*
 * THEOREMA DA LEITURA CONTIDA
 * Proposito: transportar ao destino uma região inteira do reservatório.
 * Pre-condições: meio vivo, destino não nulo e intervallo válido.
 * Effeitos: escreve no destino; o meio permanece immutável.
 * Retorno: unidade no êxito e zero sem copiar na recusa.
 * Razão: o lemma antecede a conversão do deslocamento á medida nativa.
 */
int ler_meio_simulado(const struct meio_simulado *meio, uint64_t deslocamento,
                      void *destino, uint32_t quantidade_de_bytes)
{
    if (destino == 0 || !intervallo_do_meio_e_valido(
            meio, deslocamento, quantidade_de_bytes)) {
        return 0;
    }
    memcpy(destino, meio->memoria + (size_t)deslocamento,
           (size_t)quantidade_de_bytes);
    return 1;
}

/*
 * THEOREMA DA ESCRIPTA CONTIDA
 * Proposito: transportar uma região da origem para o reservatório.
 * Pre-condições: meio vivo, origem não nula e intervallo válido.
 * Effeitos: escreve somente na região consentida do meio.
 * Retorno: unidade no êxito e zero sem copiar na recusa.
 * Razão: leitura e escripta submettem-se á mesma fronteira geométrica.
 */
int escrever_meio_simulado(struct meio_simulado *meio, uint64_t deslocamento,
                           const void *origem,
                           uint32_t quantidade_de_bytes)
{
    if (origem == 0 || !intervallo_do_meio_e_valido(
            meio, deslocamento, quantidade_de_bytes)) {
        return 0;
    }
    memcpy(meio->memoria + (size_t)deslocamento, origem,
           (size_t)quantidade_de_bytes);
    return 1;
}
