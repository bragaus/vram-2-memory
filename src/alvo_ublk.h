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
 * Proposito: fornecer memória CPU fixada por etiqueta á libublksrv.
 * Pre-condições: tamanho positivo; fila e etiqueta são contexto exterior.
 * Effeitos: adquire cudaHostAllocPortable. Retorno: endereço ou nulo.
 * Razão: somente memória fixada sustenta a promessa de DMA assíncrono.
 */
void *reservar_memoria_ublk_cuda(const struct ublksrv_queue *fila_exterior,
                                 int etiqueta, int tamanho);

/*
 * Proposito: restituir a memória fixada quando a fila exterior termina.
 * Pre-condições: endereço entregue pela reserva CUDA.
 * Effeitos: chama cudaFreeHost. Retorno: nenhum.
 * Razão: a bibliotheca impõe esta assinatura exterior sem resultado.
 */
void destruir_memoria_ublk_cuda(const struct ublksrv_queue *fila_exterior,
                                void *memoria, int etiqueta);

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
