#define _POSIX_C_SOURCE 200809L

#include "alvo_ublk.h"

#include <stdint.h>
#include <time.h>

/*
 * LEMMA DO INSTANTE MONOTONICO
 * Proposito: medir a marcha da requisição sem sujeição ao calendário civil.
 * Pre-condições: o systema fornece CLOCK_MONOTONIC.
 * Effeitos: consulta o relógio. Retorno: nanossegundos, ou zero na falha.
 * Razão: segundos são cercados antes do producto que os converte.
 */
uint64_t ler_instante_monotonico(void)
{
    struct timespec instante;

    if (clock_gettime(CLOCK_MONOTONIC, &instante) != 0 ||
        (uint64_t)instante.tv_sec > UINT64_MAX / 1000000000ULL) {
        return 0;
    }
    return (uint64_t)instante.tv_sec * 1000000000ULL +
           (uint64_t)instante.tv_nsec;
}
