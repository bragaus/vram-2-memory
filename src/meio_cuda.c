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

/*
 * COROLLARIO DA RESTITUICAO DA VRAM
 * Proposito: devolver ao engenho exacto a reserva que elle concedeu.
 * Pre-condições: nenhuma; meio nulo ou vazio é termo regular.
 * Effeitos: escolhe a GPU, liberta VRAM e zera o registro.
 * Retorno: unidade no termo ou zero se CUDA conservar a posse.
 * Razão: o registro só se apaga depois da confirmação de cudaFree.
 */
int destruir_meio_cuda(struct meio_cuda *meio)
{
    if (meio == 0 || meio->memoria_da_gpu == 0) {
        return 1;
    }
    if (cudaSetDevice(meio->indice_da_gpu) != cudaSuccess ||
        cudaFree(meio->memoria_da_gpu) != cudaSuccess) {
        return 0;
    }
    meio->memoria_da_gpu = 0;
    meio->capacidade_em_bytes = 0;
    meio->indice_da_gpu = 0;
    return 1;
}

/*
 * THEOREMA DA CORRENTE EXCLUSIVA
 * Proposito: preparar uma corrente não bloqueante para uma só fila.
 * Pre-condições: transportador vazio e meio vivo.
 * Effeitos: escolhe a GPU e adquire cudaStream_t.
 * Retorno: unidade no êxito ou zero sem estado parcial.
 * Razão: a corrente ordinária introduziria dependências entre filas irmãs.
 */
int criar_transportador_cuda(struct transportador_cuda *transportador,
                             struct meio_cuda *meio)
{
    cudaStream_t corrente;

    if (transportador == 0 || transportador->corrente != 0 || meio == 0 ||
        meio->memoria_da_gpu == 0) {
        return 0;
    }
    if (cudaSetDevice(meio->indice_da_gpu) != cudaSuccess ||
        cudaStreamCreateWithFlags(&corrente, cudaStreamNonBlocking) !=
            cudaSuccess) {
        return 0;
    }
    transportador->meio = meio;
    transportador->corrente = corrente;
    return 1;
}

/*
 * COROLLARIO DO TERMO DA CORRENTE
 * Proposito: esperar toda cópia e restituir a corrente á GPU.
 * Pre-condições: nenhuma; transportador vazio é termo regular.
 * Effeitos: synchroniza, destrói e zera o transportador.
 * Retorno: unidade somente quando synchronização e destruição concordam.
 * Razão: mesmo erro tardio não dispensa a tentativa de restituir a posse.
 */
int destruir_transportador_cuda(struct transportador_cuda *transportador)
{
    cudaError_t resultado_da_synchronizacao;
    cudaError_t resultado_da_destruicao;

    if (transportador == 0 || transportador->corrente == 0) {
        return 1;
    }
    if (transportador->meio == 0 ||
        cudaSetDevice(transportador->meio->indice_da_gpu) != cudaSuccess) {
        return 0;
    }
    resultado_da_synchronizacao =
        cudaStreamSynchronize(transportador->corrente);
    resultado_da_destruicao = cudaStreamDestroy(transportador->corrente);
    if (resultado_da_destruicao == cudaSuccess) {
        transportador->corrente = 0;
        transportador->meio = 0;
    }
    return resultado_da_synchronizacao == cudaSuccess &&
           resultado_da_destruicao == cudaSuccess;
}
