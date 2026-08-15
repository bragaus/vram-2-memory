#ifndef MEIO_SIMULADO_H
#define MEIO_SIMULADO_H

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

#endif
