#include "../src/meio_cuda.h"

/*
 * PROVA DA TRAVESSIA CUDA
 * Proposito: confrontar escripta e leitura byte a byte por memória fixada.
 * Pre-condições: GPU de índice zero e 8192 octetos de VRAM disponíveis.
 * Effeitos: reserva VRAM, corrente e dois buffers CPU fixados.
 * Retorno: zero na convergência e unidade na primeira divergência.
 * Razão: padrões distinctos denunciam omissão, troca ou invasão no DMA.
 */
int main(void)
{
    struct meio_cuda meio = {0};
    struct transportador_cuda transportador = {0};
    unsigned char *origem = 0;
    unsigned char *destino = 0;
    int resultado = 1;

    if (!criar_meio_cuda(&meio, 0, 8192) ||
        !criar_transportador_cuda(&transportador, &meio)) goto termo;
    origem = reservar_memoria_intermediaria_cuda(4096);
    destino = reservar_memoria_intermediaria_cuda(4096);
    if (origem == 0 || destino == 0) goto termo;
    for (unsigned int indice = 0; indice < 4096; indice++) {
        origem[indice] = (unsigned char)(indice * 29U + 7U);
        destino[indice] = 0;
    }
    if (!escrever_meio_cuda(&transportador, 2048, origem, 4096) ||
        !ler_meio_cuda(&transportador, 2048, destino, 4096)) goto termo;
    for (unsigned int indice = 0; indice < 4096; indice++) {
        if (origem[indice] != destino[indice]) goto termo;
    }
    if (ler_meio_cuda(&transportador, 4097, destino, 4096) ||
        escrever_meio_cuda(&transportador, 8192, origem, 1)) goto termo;
    resultado = 0;

termo:
    if (!destruir_memoria_intermediaria_cuda(destino)) resultado = 1;
    if (!destruir_memoria_intermediaria_cuda(origem)) resultado = 1;
    if (!destruir_transportador_cuda(&transportador)) resultado = 1;
    if (!destruir_meio_cuda(&meio)) resultado = 1;
    return resultado;
}
