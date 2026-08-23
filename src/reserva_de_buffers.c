#include "reserva_de_buffers.h"
#include "configuracao.h"
#include "plano_da_memoria.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * THEOREMA DA RESERVA ALINHADA INTEGRAL
 * Proposito: adquirir de uma vez todos os buffers da geometria.
 * Pre-condições: reserva vazia, factores positivos e operação alinhada.
 * Effeitos: adquire e zera uma região; publica a figura somente no êxito.
 * Retorno: zero, erro de domínio, transbordamento ou falta de memória.
 * Razão: aligned_alloc antecede o trabalho e dá base certa ao registro CUDA.
 */
int criar_reserva_de_buffers(struct reserva_de_buffers *reserva,
                             int quantidade_de_filas,
                             int profundidade_das_filas,
                             uint32_t tamanho_do_buffer)
{
    struct configuracao_do_apparelho configuracao = {0};
    uint64_t quantidade_em_bytes;
    unsigned char *inicio;
    int resultado;

    if (reserva == 0 || reserva->inicio != 0 || quantidade_de_filas <= 0 ||
        profundidade_das_filas <= 0 || tamanho_do_buffer == 0 ||
        tamanho_do_buffer % TAMANHO_DO_BLOCO_EM_BYTES != 0) return -EINVAL;
    configuracao.quantidade_de_filas = quantidade_de_filas;
    configuracao.profundidade_das_filas = profundidade_das_filas;
    configuracao.maior_operacao_em_bytes = tamanho_do_buffer;
    resultado = calcular_memoria_intermediaria(
        &configuracao, &quantidade_em_bytes);
    if (resultado < 0) return resultado;
    if (quantidade_em_bytes > SIZE_MAX) return -EOVERFLOW;
    inicio = aligned_alloc(TAMANHO_DO_BLOCO_EM_BYTES,
                           (size_t)quantidade_em_bytes);
    if (inicio == 0) return -ENOMEM;
    memset(inicio, 0, (size_t)quantidade_em_bytes);
    reserva->inicio = inicio;
    reserva->quantidade_em_bytes = quantidade_em_bytes;
    reserva->tamanho_do_buffer = tamanho_do_buffer;
    reserva->quantidade_de_filas = quantidade_de_filas;
    reserva->profundidade_das_filas = profundidade_das_filas;
    return 0;
}

/*
 * LEMMA DO QUINHAO NUMERADO
 * Proposito: achar o buffer singular de uma fila e etiqueta.
 * Pre-condições: nenhuma; toda grandeza exterior será cercada.
 * Effeitos: nenhum. Retorno: início do quinhão ou nulo na recusa.
 * Razão: a ordem fila vezes profundidade mais etiqueta é determinística.
 */
void *achar_buffer_reservado(const struct reserva_de_buffers *reserva,
                             int indice_da_fila, int etiqueta, int tamanho)
{
    uint64_t ordem;
    uint64_t deslocamento;

    if (reserva == 0 || reserva->inicio == 0 || indice_da_fila < 0 ||
        indice_da_fila >= reserva->quantidade_de_filas || etiqueta < 0 ||
        etiqueta >= reserva->profundidade_das_filas || tamanho <= 0 ||
        (uint32_t)tamanho > reserva->tamanho_do_buffer) return 0;
    ordem = (uint64_t)indice_da_fila *
        (uint64_t)reserva->profundidade_das_filas + (uint64_t)etiqueta;
    deslocamento = ordem * reserva->tamanho_do_buffer;
    if (deslocamento > reserva->quantidade_em_bytes ||
        reserva->tamanho_do_buffer >
            reserva->quantidade_em_bytes - deslocamento) return 0;
    return reserva->inicio + (size_t)deslocamento;
}

/*
 * COROLLARIO DA RESTITUICAO INTEGRAL
 * Proposito: devolver a região única e apagar sua geometria.
 * Pre-condições: nenhum registro exterior permanece sobre a memória.
 * Effeitos: chama free e reduz todos os campos a zero.
 * Retorno: nenhum. Razão: a figura vazia torna o termo repetido regular.
 */
void destruir_reserva_de_buffers(struct reserva_de_buffers *reserva)
{
    if (reserva == 0) return;
    free(reserva->inicio);
    reserva->inicio = 0;
    reserva->quantidade_em_bytes = 0;
    reserva->tamanho_do_buffer = 0;
    reserva->quantidade_de_filas = 0;
    reserva->profundidade_das_filas = 0;
}
