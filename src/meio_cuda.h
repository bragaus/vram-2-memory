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

/*
 * Proposito: reservar memória CPU fixada para que DMA a possa atravessar.
 * Pre-condições: quantidade positiva e alojável.
 * Effeitos: adquire cudaHostAllocPortable. Retorno: endereço ou nulo.
 * Razão: memória paginável não demonstra travessia assíncrona por DMA.
 */
void *reservar_memoria_intermediaria_cuda(uint32_t quantidade_de_bytes);

/*
 * Proposito: devolver a memória CPU fixada depois da última transferência.
 * Pre-condições: endereço nasceu de cudaHostAlloc ou é nulo.
 * Effeitos: chama cudaFreeHost. Retorno: unidade no êxito ou zero.
 * Razão: o nulo representa posse nenhuma e tem termo regular.
 */
int destruir_memoria_intermediaria_cuda(void *memoria);

/*
 * Proposito: copiar VRAM para memória CPU fixada pela corrente da fila.
 * Pre-condições: região contida e destino nascido de cudaHostAlloc.
 * Effeitos: submette DMA e espera sua conclusão. Retorno: unidade ou zero.
 * Razão: a espera antecede a conclusão ublk que reutilizará o buffer.
 */
int ler_meio_cuda(struct transportador_cuda *transportador,
                  uint64_t deslocamento, void *destino,
                  uint32_t quantidade_de_bytes);

/*
 * Proposito: copiar memória CPU fixada para VRAM pela corrente da fila.
 * Pre-condições: região contida e origem nascida de cudaHostAlloc.
 * Effeitos: submette DMA e espera sua conclusão. Retorno: unidade ou zero.
 * Razão: a espera torna a conclusão do bloco posterior á transferência.
 */
int escrever_meio_cuda(struct transportador_cuda *transportador,
                       uint64_t deslocamento, const void *origem,
                       uint32_t quantidade_de_bytes);

/*
 * Proposito: reduzir a zero uma região da VRAM pela corrente da fila.
 * Pre-condições: transportador vivo e intervallo contido.
 * Effeitos: submette cudaMemsetAsync e espera. Retorno: unidade ou zero.
 * Razão: descarte não reclama buffer CPU nem abandona dados recuperáveis.
 */
int zerar_meio_cuda(struct transportador_cuda *transportador,
                    uint64_t deslocamento, uint32_t quantidade_de_bytes);

#endif
