#ifndef ALVO_UBLK_H
#define ALVO_UBLK_H

#include "contrato_do_meio.h"
#include "fila_de_requisicoes.h"
#include "retrato_do_observatorio.h"

#include <ublksrv.h>

/* A fila exterior encontra aqui o meio e o livro de suas etiquetas. */
struct contexto_da_fila_ublk {
    struct fila_de_requisicoes *fila;
    const struct operacoes_do_meio *operacoes_do_meio;
    void *contexto_do_meio;
    int indice_da_fila;
    struct contadores_da_fila *contadores;
    uint64_t prazo_em_nanossegundos;
    int resultado_assincrono;
};

/*
 * Proposito: obter instante monotónico para abrir o prazo da requisição.
 * Pre-condições: o systema fornece CLOCK_MONOTONIC.
 * Effeitos: consulta o relógio. Retorno: nanossegundos, ou zero na falha.
 * Razão: o calendário civil jámais governa o vencimento das filas.
 */
uint64_t ler_instante_monotonico(void);

/*
 * Proposito: applicar uma operação ublk ao contracto commum do meio.
 * Pre-condições: taboa, contexto e região previamente cercados.
 * Effeitos: submette e promette conclusão, sem colher a corrente.
 * Retorno: zero na promessa, um no acto immediato ou erro sem promessa.
 * Razão: a fila colherá o evento e concluirá pelo registro persistente.
 */
int transferir_requisicao_ublk(struct contexto_da_fila_ublk *contexto,
                               uint8_t operacao, uint32_t etiqueta,
                               uint64_t deslocamento, void *memoria,
                               uint32_t quantidade_de_bytes);

/*
 * Proposito: confrontar prazo, entregar resultado e restituir etiqueta.
 * Pre-condições: etiqueta transferindo e instante final monotónico.
 * Effeitos: conclue uma vez ou falla e ordena parada do dispositivo.
 * Retorno: resultado da entrega exterior ou erro negativo do prazo.
 * Razão: a etiqueta só regressa á espera depois da aceitação do núcleo.
 */
int entregar_requisicao_ublk(struct contexto_da_fila_ublk *contexto,
                             const struct ublksrv_queue *fila_exterior,
                             uint32_t etiqueta, int resultado,
                             uint64_t instante_final);

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
