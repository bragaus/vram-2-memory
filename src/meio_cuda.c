#include "meio_cuda.h"

#include <stddef.h>
#include <stdint.h>

/*
 * THEOREMA DA RESERVA NA GPU
 * Proposito: escolher o engenho, adquirir VRAM e zerar toda a extensão.
 * Pre-condições: meio vazio, índice não negativo e capacidade alojável.
 * Effeitos: altera o dispositivo CUDA corrente e estabelece a reserva.
 * Retorno: unidade no êxito; zero com libertação na primeira falha.
 * Razão: o zero inicial impede que leitura preceda escripta com lixo antigo.
 */
int criar_meio_cuda(struct meio_cuda *meio, int indice_da_gpu,
                    uint64_t capacidade_em_bytes)
{
    unsigned char *memoria;

    if (meio == 0 || meio->memoria_da_gpu != 0 || indice_da_gpu < 0 ||
        capacidade_em_bytes == 0 || capacidade_em_bytes > SIZE_MAX) {
        return 0;
    }
    if (cudaSetDevice(indice_da_gpu) != cudaSuccess ||
        cudaMalloc((void **)&memoria, (size_t)capacidade_em_bytes) !=
            cudaSuccess) {
        return 0;
    }
    if (cudaMemset(memoria, 0, (size_t)capacidade_em_bytes) != cudaSuccess) {
        cudaFree(memoria);
        return 0;
    }
    meio->memoria_da_gpu = memoria;
    meio->capacidade_em_bytes = capacidade_em_bytes;
    meio->indice_da_gpu = indice_da_gpu;
    return 1;
}
