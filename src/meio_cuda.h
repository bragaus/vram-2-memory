#ifndef MEIO_CUDA_H
#define MEIO_CUDA_H
#include <cuda_runtime_api.h>
#include <stdint.h>
/* O reservatório reside na GPU; sua capacidade limita todo deslocamento. */
struct meio_cuda {
    unsigned char *memoria_da_gpu;
    uint64_t capacidade_em_bytes;
    int indice_da_gpu;
};
/* Cada fila possue corrente CUDA própria sobre o reservatório commum. */
struct transportador_cuda {
    struct meio_cuda *meio;
    cudaStream_t corrente;
};
/*
 * Proposito: escolher a GPU e reservar toda a VRAM antes da publicação.
 * Pre-condições: meio vazio, índice não negativo e capacidade alojável.
 * Effeitos: fixa o dispositivo e adquire VRAM. Retorno: unidade ou zero.
 * Razão: nenhuma reserva CUDA poderá nascer no caminho crítico.
 */
int criar_meio_cuda(struct meio_cuda *meio, int indice_da_gpu,
                    uint64_t capacidade_em_bytes);

/*
 * Proposito: devolver a VRAM e reduzir o meio á figura vazia.
 * Pre-condições: nenhuma; meio nulo ou vazio não produz effeito.
 * Effeitos: chama cudaFree no dispositivo possuidor. Retorno: unidade ou zero.
 * Razão: o índice é restaurado antes de tocar a reserva correspondente.
 */
int destruir_meio_cuda(struct meio_cuda *meio);

/*
 * Proposito: crear corrente não bloqueante exclusiva para uma fila.
 * Pre-condições: meio vivo e transportador vazio.
 * Effeitos: adquire cudaStream_t. Retorno: unidade ou zero.
 * Razão: filas independentes não hão de serializar na corrente ordinária.
 */
int criar_transportador_cuda(struct transportador_cuda *transportador,
                             struct meio_cuda *meio);

/*
 * Proposito: destruir a corrente depois de todas as cópias concluídas.
 * Pre-condições: nenhuma. Effeitos: synchroniza e restitue a corrente.
 * Retorno: unidade no êxito e zero se CUDA recusar o termo.
 * Razão: jámais se liberta meio ainda referenciado por operação assíncrona.
 */
int destruir_transportador_cuda(struct transportador_cuda *transportador);

#endif
