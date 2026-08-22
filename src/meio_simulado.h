#ifndef MEIO_SIMULADO_H
#define MEIO_SIMULADO_H

#include "contrato_do_meio.h"

#include <stdint.h>

/* Reservatório ordinário que representa, sem GPU, a memória do apparelho. */
struct meio_simulado {
    unsigned char *memoria;
    uint64_t capacidade_em_bytes;
};

/*
 * Proposito: reservar e zerar o reservatório da capacidade pedida.
 * Pre-condições: meio não nulo, ainda vazio, e capacidade representável.
 * Effeitos: adquire memória. Retorno: unidade, ou zero sem alterar o meio.
 * Razão: a reserva antecede a publicação e exclue a pressão tardia.
 */
int criar_meio_simulado(struct meio_simulado *meio,
                        uint64_t capacidade_em_bytes);

/*
 * Proposito: devolver ao systema a reserva do meio.
 * Pre-condições: nenhuma; o ponteiro poderá ser nulo.
 * Effeitos: liberta e zera o registro. Retorno: nenhum.
 * Razão: a figura vazia torna segura a destruição repetida.
 */
void destruir_meio_simulado(struct meio_simulado *meio);

/*
 * Proposito: copiar do reservatório para a memória do consulente.
 * Pre-condições: meio vivo, destino não nulo e intervallo contido.
 * Effeitos: altera somente o destino. Retorno: unidade ou zero.
 * Razão: a subtracção de limites evita somma susceptível a transbordar.
 */
int ler_meio_simulado(const struct meio_simulado *meio, uint64_t deslocamento,
                      void *destino, uint32_t quantidade_de_bytes);

/*
 * Proposito: copiar da memória do consulente para o reservatório.
 * Pre-condições: meio vivo, origem não nula e intervallo contido.
 * Effeitos: altera somente o reservatório. Retorno: unidade ou zero.
 * Razão: a mesma geometria rege leitura e escripta.
 */
int escrever_meio_simulado(struct meio_simulado *meio, uint64_t deslocamento,
                           const void *origem, uint32_t quantidade_de_bytes);

/*
 * Proposito: reduzir a zero uma região dispensada ou explicitamente zerada.
 * Pre-condições: meio vivo e intervallo contido.
 * Effeitos: altera somente a região. Retorno: unidade ou zero.
 * Razão: descarte e escripta de zeros possuem o mesmo effeito no meio volátil.
 */
int zerar_meio_simulado(struct meio_simulado *meio, uint64_t deslocamento,
                        uint32_t quantidade_de_bytes);

/*
 * Proposito: revelar a taboa assíncrona do reservatório ordinário.
 * Pre-condições: nenhuma; a taboa possue duração estática.
 * Effeitos: nenhum. Retorno: operações immutáveis do meio simulado.
 * Razão: o alvo governa o contracto sem conhecer a matéria subjacente.
 */
const struct operacoes_do_meio *obter_operacoes_do_meio_simulado(void);

/*
 * Proposito: adquirir o contexto assíncrono do meio simulado.
 * Pre-condições: destino vazio e configuração integralmente válida.
 * Effeitos: reserva o meio e uma conclusão por fila. Retorno: zero ou -errno.
 * Razão: toda posse nasce antes que a taboa seja entregue ao alvo.
 */
int preparar_meio_assincrono_simulado(
    void **contexto, const struct configuracao_do_apparelho *configuracao);

/*
 * Proposito: restituir o contexto assíncrono integralmente.
 * Pre-condições: nenhuma conclusão permanece pendente.
 * Effeitos: liberta reservatório, escrivaninhas e invólucro.
 * Retorno: nenhum. Razão: a posse exterior converge numa porta singular.
 */
void destruir_meio_assincrono_simulado(void *contexto);

/*
 * Proposito: reconhecer a fila que colherá suas próprias sentenças.
 * Pre-condições: contexto vivo e índice pertencente á configuração.
 * Effeitos: nenhum no simulador. Retorno: zero ou -EINVAL.
 * Razão: conservar a mesma fronteira exigida pelos contextos CUDA.
 */
int vincular_fila_do_meio_simulado(void *contexto, int indice_da_fila);

#endif
