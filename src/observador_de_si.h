#ifndef OBSERVADOR_DE_SI_H
#define OBSERVADOR_DE_SI_H

#include "retrato_do_observatorio.h"

#include <stddef.h>
#include <stdint.h>
enum certeza_da_narracao {
    CERTEZA_OBSERVEI,
    CERTEZA_INFERI,
    CERTEZA_IGNORO
};

enum modo_da_narracao {
    MODO_DA_NARRACAO_SILENCIOSO,
    MODO_DA_NARRACAO_SOBRIO,
    MODO_DA_NARRACAO_THEATRAL
};

struct configuracao_da_narracao {
    enum modo_da_narracao modo;
    uint64_t idade_maxima_em_nanossegundos;
    uint64_t p99_alarmante_em_microssegundos;
};

/*
 * Proposito: julgar quanto o observador pode affirmar sobre seu retrato.
 * Pre-condições: retrato e configuração válidos; instante presente monotónico.
 * Effeitos: nenhum. Retorno: observação, inferência ou ignorância.
 * Razão: idade e presença precedem qualquer interpretação das grandezas.
 */
enum certeza_da_narracao julgar_certeza_do_observador(
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_da_narracao *configuracao,
    uint64_t instante_presente_em_nanossegundos);

/*
 * Proposito: dar primeira pessoa honesta ao estado do apparelho.
 * Pre-condições: argumentos válidos e certeza previamente julgada.
 * Effeitos: escreve texto fixo e grandezas do retrato, nunca texto exterior.
 * Retorno: octetos visíveis, zero no silêncio e SIZE_MAX na recusa.
 * Razão: modos mudam ornamento, mas certeza e causas permanecem invariantes.
 */
size_t narrar_observador_de_si(
    char *destino, size_t capacidade,
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_da_narracao *configuracao,
    uint64_t instante_presente_em_nanossegundos);

#endif
