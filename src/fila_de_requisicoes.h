#ifndef FILA_DE_REQUISICOES_H
#define FILA_DE_REQUISICOES_H
#include "estado_da_requisicao.h"
/* Cada fila possui um escriptorio exclusivo para cada etiqueta ublk. */
struct fila_de_requisicoes {
    struct registro_da_requisicao *registros;
    uint32_t profundidade;
};
/*
 * Proposito: preparar todos os registros antes de publicar a fila.
 * Pre-condições: fila vazia e profundidade positiva representável.
 * Effeitos: reserva registros no estado aguardando.
 * Retorno: unidade no êxito, zero sem estado parcial na falha.
 * Razão: nenhuma reserva deverá occorrer no caminho crítico.
 */
int criar_fila_de_requisicoes(struct fila_de_requisicoes *fila,
                              uint32_t profundidade);

/*
 * Proposito: restituir os registros e reduzir a fila ao vazio.
 * Pre-condições: nenhuma; a fila poderá ser nula ou já vazia.
 * Effeitos: liberta a reserva. Retorno: nenhum.
 * Razão: zerar a figura torna idempotente o termo da fila.
 */
void destruir_fila_de_requisicoes(struct fila_de_requisicoes *fila);

/*
 * Proposito: entregar uma etiqueta aguardando a uma única operação.
 * Pre-condições: etiqueta contida; operações sem carga poderão medir zero.
 * Effeitos: registra grandezas e passa ao estado transferindo.
 * Retorno: ponteiro exclusivo, ou nulo se a etiqueta não repousava.
 * Razão: a mudança prévia de estado prohibe duas posses simultâneas.
 */
struct registro_da_requisicao *iniciar_requisicao_na_fila(
    struct fila_de_requisicoes *fila, uint32_t etiqueta,
    uint64_t deslocamento, uint32_t quantidade_de_bytes,
    uint8_t operacao, void *memoria_intermediaria,
    uint64_t instante_inicial_em_nanossegundos);

/*
 * Proposito: concluir uma vez a transferência indicada pela etiqueta.
 * Pre-condições: registro transferindo e resultado conhecido.
 * Effeitos: grava resultado e conserva a etiqueta em conclusão.
 * Retorno: unidade na primeira conclusão e zero nas repetições.
 * Razão: somente transferindo atravessa o umbral da conclusão.
 */
int concluir_requisicao_na_fila(struct fila_de_requisicoes *fila,
                                uint32_t etiqueta, int resultado);

/*
 * Proposito: restituir a etiqueta depois da entrega exterior confirmada.
 * Pre-condições: registro concluindo. Effeitos: passa a aguardando.
 * Retorno: unidade na restituição e zero fora d'esse estado.
 * Razão: jamais se reutiliza etiqueta antes da confirmação do núcleo.
 */
int rearmar_requisicao_na_fila(struct fila_de_requisicoes *fila,
                               uint32_t etiqueta);

/*
 * Proposito: condemnar transferência cujo prazo conhecido se consumiu.
 * Pre-condições: relógio monotónico e prazo positivo em nanossegundos.
 * Effeitos: passa de transferindo a falhou e grava o resultado.
 * Retorno: unidade somente quando o vencimento é demonstrado.
 * Razão: ordenar os instantes precede a subtracção e evita retorno do relógio.
 */
int falhar_requisicao_vencida(struct fila_de_requisicoes *fila,
                              uint32_t etiqueta, uint64_t instante_actual,
                              uint64_t prazo, int resultado);

#endif
