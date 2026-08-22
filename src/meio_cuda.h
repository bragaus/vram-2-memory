#ifndef MEIO_CUDA_H
#define MEIO_CUDA_H
#include "contrato_do_meio.h"
#include <cuda.h>
#include <stdint.h>

struct ublksrv_queue;
/* O reservatório reside na GPU; sua capacidade limita todo deslocamento. */
struct meio_cuda {
    CUdeviceptr memoria_da_gpu;
    CUcontext contexto;
    CUdevice dispositivo;
    uint64_t capacidade_em_bytes;
};
/* Cada fila possue corrente CUDA própria sobre o reservatório commum. */
struct transportador_cuda {
    struct meio_cuda *meio;
    CUstream corrente;
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
 * Effeitos: chama cuMemFree no dispositivo possuidor. Retorno: unidade ou zero.
 * Razão: o índice é restaurado antes de tocar a reserva correspondente.
 */
int destruir_meio_cuda(struct meio_cuda *meio);

/*
 * Proposito: crear corrente não bloqueante exclusiva para uma fila.
 * Pre-condições: meio vivo e transportador vazio.
 * Effeitos: adquire CUstream. Retorno: unidade ou zero.
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
 * Pre-condições: contexto CUDA corrente; quantidade positiva e alojável.
 * Effeitos: adquire CU_MEMHOSTALLOC_PORTABLE. Retorno: endereço ou nulo.
 * Razão: memória paginável não demonstra travessia assíncrona por DMA.
 */
void *reservar_memoria_intermediaria_cuda(uint32_t quantidade_de_bytes);

/*
 * Proposito: adaptar a reserva CUDA á assinatura exterior de libublksrv.
 * Pre-condições: tamanho positivo; fila e etiqueta poderão ser ignoradas.
 * Effeitos: adquire memória CPU fixada. Retorno: endereço ou nulo.
 * Razão: a adaptação pertence ao meio, não ao alvo que o empregará.
 */
void *reservar_memoria_exterior_cuda(
    const struct ublksrv_queue *fila_exterior, int etiqueta, int tamanho);

/*
 * Proposito: devolver a memória CPU fixada depois da última transferência.
 * Pre-condições: contexto corrente; endereço nasceu de cuMemHostAlloc ou nulo.
 * Effeitos: chama cuMemFreeHost. Retorno: unidade no êxito ou zero.
 * Razão: o nulo representa posse nenhuma e tem termo regular.
 */
int destruir_memoria_intermediaria_cuda(void *memoria);

/*
 * Proposito: adaptar a restituição CUDA á assinatura de libublksrv.
 * Pre-condições: memória fixada ou nula; fila e etiqueta são exteriores.
 * Effeitos: restitue a região. Retorno: nenhum.
 * Razão: o meio encerra a posse material que elle proprio concedeu.
 */
void destruir_memoria_exterior_cuda(
    const struct ublksrv_queue *fila_exterior, void *memoria, int etiqueta);

/*
 * Proposito: copiar VRAM para memória CPU fixada pela corrente da fila.
 * Pre-condições: região contida e destino nascido de cuMemHostAlloc.
 * Effeitos: submette DMA e espera sua conclusão. Retorno: unidade ou zero.
 * Razão: a espera antecede a conclusão que reutilizará a memória.
 */
int ler_meio_cuda(struct transportador_cuda *transportador,
                  uint64_t deslocamento, void *destino,
                  uint32_t quantidade_de_bytes);

/*
 * Proposito: copiar memória CPU fixada para VRAM pela corrente da fila.
 * Pre-condições: região contida e origem nascida de cuMemHostAlloc.
 * Effeitos: submette DMA e espera sua conclusão. Retorno: unidade ou zero.
 * Razão: a espera torna a conclusão do bloco posterior á transferência.
 */
int escrever_meio_cuda(struct transportador_cuda *transportador,
                       uint64_t deslocamento, const void *origem,
                       uint32_t quantidade_de_bytes);

/*
 * Proposito: reduzir a zero uma região da VRAM pela corrente da fila.
 * Pre-condições: transportador vivo e intervallo contido.
 * Effeitos: submette cuMemsetD8Async e espera. Retorno: unidade ou zero.
 * Razão: descarte não reclama memória CPU nem abandona dados recuperáveis.
 */
int zerar_meio_cuda(struct transportador_cuda *transportador,
                    uint64_t deslocamento, uint32_t quantidade_de_bytes);

/*
 * Proposito: revelar a taboa assíncrona do reservatório CUDA.
 * Pre-condições: nenhuma; a taboa possue duração estática.
 * Effeitos: nenhum. Retorno: operações immutáveis do meio CUDA.
 * Razão: a lei commum encobre a execução material hoje synchrona.
 */
const struct operacoes_do_meio *obter_operacoes_do_meio_cuda(void);

#endif
