#ifndef ALVO_UBLK_H
#define ALVO_UBLK_H

#include "fila_de_requisicoes.h"
#include "meio_simulado.h"

#include <ublksrv.h>

/* A fila exterior encontra aqui o meio e o livro de suas etiquetas. */
struct contexto_da_fila_ublk {
    struct fila_de_requisicoes *fila;
    struct meio_simulado *meio;
    uint64_t prazo_em_nanossegundos;
};

/*
 * Proposito: obter instante monotónico para abrir o prazo da requisição.
 * Pre-condições: o systema fornece CLOCK_MONOTONIC.
 * Effeitos: consulta o relógio. Retorno: nanossegundos, ou zero na falha.
 * Razão: o calendário civil jámais governa o vencimento das filas.
 */
uint64_t ler_instante_monotonico(void);

/*
 * Proposito: applicar uma operação ublk ao meio simulado.
 * Pre-condições: contexto vivo e região previamente cercada.
 * Effeitos: lê, escreve ou confirma descarga síncrona.
 * Retorno: octetos transportados, zero na descarga ou erro negativo.
 * Razão: operações externas convergem numa sentença autoral verificável.
 */
int transferir_requisicao_ublk(struct contexto_da_fila_ublk *contexto,
                               uint8_t operacao, uint64_t deslocamento,
                               void *memoria, uint32_t quantidade_de_bytes);

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
