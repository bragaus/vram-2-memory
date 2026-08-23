#ifndef CONTRATO_DO_MEIO_H
#define CONTRATO_DO_MEIO_H

#include "configuracao.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Proposito: receber exactamente uma sentença depois da operação acceita.
 * Pre-condições: argumento conserva vida até a colheita da fila proprietária.
 * Effeitos: entrega zero ou erro negativo. Retorno: nenhum.
 * Razão: separar submissão e conclusão permitte ao DMA marchar sem espera.
 */
typedef void (*funcao_de_conclusao_do_meio)(void *argumento, int erro);

/*
 * TRACTADO DAS OPERAÇÕES DO MEIO
 *
 * Domínio: um contexto singular e filas numeradas pela configuração.
 * Invariante: retorno negativo de submissão prohibe conclusão ulterior;
 * retorno zero promette uma só conclusão, dada pela própria fila ao colher.
 * Effeitos: o meio poderá reservar, transportar e restituir sua matéria.
 */
struct operacoes_do_meio {
    int (*preparar)(void **contexto,
                    const struct configuracao_do_apparelho *configuracao);
    int (*vincular_fila)(void *contexto, int indice_da_fila);
    int (*aquecer_fila)(void *contexto, int indice_da_fila,
                        void *memoria, size_t quantidade_de_bytes);
    void (*destruir)(void *contexto);
    int (*ler)(void *contexto, int indice_da_fila, int etiqueta,
               uint64_t deslocamento, void *destino,
               uint32_t quantidade_de_bytes,
               funcao_de_conclusao_do_meio concluir, void *argumento);
    int (*escrever)(void *contexto, int indice_da_fila, int etiqueta,
                    uint64_t deslocamento, const void *origem,
                    uint32_t quantidade_de_bytes,
                    funcao_de_conclusao_do_meio concluir, void *argumento);
    int (*zerar)(void *contexto, int indice_da_fila, int etiqueta,
                 uint64_t deslocamento, uint32_t quantidade_de_bytes,
                 funcao_de_conclusao_do_meio concluir, void *argumento);
    int (*colher)(void *contexto, int indice_da_fila, int orcamento);
};

#endif
