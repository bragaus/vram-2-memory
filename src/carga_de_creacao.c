#include "carga_de_creacao.h"

#include <errno.h>
#include <stdint.h>

/*
 * THEOREMA DA CARGA EXTERIOR
 * Proposito: escrever os seis campos como sete palavras de trinta e dois bits.
 * Pre-condições: figura válida e destino bastante. Effeitos: grava tudo.
 * Retorno: zero, -EINVAL ou -ENOBUFS, sem escripta nos erros.
 * Razão: palavras maiores primeiro independem da representação da machina.
 */
int escrever_carga_de_creacao(unsigned char *destino, size_t capacidade,
                              const struct configuracao_do_apparelho *figura)
{
    uint32_t campos[7];
    size_t indice;

    if (destino == 0 || !configuracao_do_apparelho_e_valida(figura))
        return -EINVAL;
    if (capacidade < TAMANHO_DA_CARGA_DE_CREACAO) return -ENOBUFS;
    campos[0] = (uint32_t)figura->indice_da_gpu;
    campos[1] = (uint32_t)(figura->capacidade_em_bytes >> 32);
    campos[2] = (uint32_t)figura->capacidade_em_bytes;
    campos[3] = (uint32_t)figura->quantidade_de_filas;
    campos[4] = (uint32_t)figura->profundidade_das_filas;
    campos[5] = figura->maior_operacao_em_bytes;
    campos[6] = figura->prazo_da_operacao_em_milissegundos;
    for (indice = 0; indice < 7; indice++) {
        destino[indice * 4] = (unsigned char)(campos[indice] >> 24);
        destino[indice * 4 + 1] = (unsigned char)(campos[indice] >> 16);
        destino[indice * 4 + 2] = (unsigned char)(campos[indice] >> 8);
        destino[indice * 4 + 3] = (unsigned char)campos[indice];
    }
    return 0;
}
