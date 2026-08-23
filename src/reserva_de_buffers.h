#ifndef RESERVA_DE_BUFFERS_H
#define RESERVA_DE_BUFFERS_H

#include <stddef.h>
#include <stdint.h>

/* A reserva única reparte RAM alinhada entre todas as filas e etiquetas. */
struct reserva_de_buffers {
    unsigned char *inicio;
    uint64_t quantidade_em_bytes;
    uint32_t tamanho_do_buffer;
    int quantidade_de_filas;
    int profundidade_das_filas;
};

/*
 * Proposito: adquirir de uma vez todos os buffers alinhados da geometria.
 * Pre-condições: reserva vazia, factores positivos e tamanho representável.
 * Effeitos: reserva e zera a região. Retorno: zero ou erro negativo.
 * Razão: nenhuma etiqueta poderá provocar nova acquisição após publicação.
 */
int criar_reserva_de_buffers(struct reserva_de_buffers *reserva,
                             int quantidade_de_filas,
                             int profundidade_das_filas,
                             uint32_t tamanho_do_buffer);

/*
 * Proposito: achar o quinhão pertencente a uma fila e etiqueta.
 * Pre-condições: nenhuma; índices e tamanhos estranhos serão recusados.
 * Effeitos: nenhum. Retorno: buffer alinhado ou nulo fora do domínio.
 * Razão: a identidade exacta substitue toda escolha por ordem de chegada.
 */
void *achar_buffer_reservado(const struct reserva_de_buffers *reserva,
                             int indice_da_fila, int etiqueta, int tamanho);

/*
 * Proposito: restituir a região única e apagar a figura da reserva.
 * Pre-condições: a memória não permanece registrada por meio exterior.
 * Effeitos: liberta e zera o registro. Retorno: nenhum.
 * Razão: todas as etiquetas perdem a posse numa única porta verificável.
 */
void destruir_reserva_de_buffers(struct reserva_de_buffers *reserva);

#endif
