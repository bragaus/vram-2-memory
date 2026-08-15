#ifndef ALVO_UBLK_H
#define ALVO_UBLK_H

#include "fila_de_requisicoes.h"
#include "meio_simulado.h"

#include <ublksrv.h>

/* A fila exterior encontra aqui o meio e o livro de suas etiquetas. */
struct contexto_da_fila_ublk {
    struct fila_de_requisicoes *fila;
    struct meio_simulado *meio;
};

/*
 * Proposito: traduzir uma requisição ublk em leitura ou escripta do meio.
 * Pre-condições: fila, contexto, etiqueta e memória intermediária válidos.
 * Effeitos: possue a etiqueta, transporta octetos e conclue-a uma vez.
 * Retorno: zero após entrega; negativo quando a entrega exterior falha.
 * Razão: a interface externa fica cercada pelo contracto autoral da fila.
 */
int tratar_requisicao_ublk(const struct ublksrv_queue *fila_exterior,
                           const struct ublk_io_data *dados);

#endif
