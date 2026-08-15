#include "observador_de_si.h"

#include <inttypes.h>
#include <stdio.h>

/*
 * THEOREMA DA CERTEZA LIMITADA
 * Proposito: escolher a autoridade exacta da próxima sentença.
 * Pre-condições: ponteiros válidos e presente pertencente ao relógio monotónico.
 * Effeitos: nenhum. Retorno: OBSERVEI, INFERI ou IGNORO.
 * Razão: validade temporal precede facto; facto precede interpretação.
 */
enum certeza_da_narracao julgar_certeza_do_observador(
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_da_narracao *configuracao,
    uint64_t instante_presente_em_nanossegundos)
{
    uint64_t idade;

    if (retrato == 0 || configuracao == 0 ||
        configuracao->idade_maxima_em_nanossegundos == 0 ||
        instante_presente_em_nanossegundos <
            retrato->instante_monotonico_em_nanossegundos)
        return CERTEZA_IGNORO;
    idade = instante_presente_em_nanossegundos -
            retrato->instante_monotonico_em_nanossegundos;
    if (idade > configuracao->idade_maxima_em_nanossegundos ||
        retrato->duracao_da_janella_em_nanossegundos == 0)
        return CERTEZA_IGNORO;
    if (retrato->operacoes_concluidas != 0 &&
        retrato->latencia_p99_em_microssegundos >=
            configuracao->p99_alarmante_em_microssegundos)
        return CERTEZA_INFERI;
    return CERTEZA_OBSERVEI;
}
/*
 * THEOREMA DA VOZ HONESTA
 * Proposito: narrar certeza e causa sem alterar o retrato contemplado.
 * Pre-condições: destino, retrato, configuração e modo válidos.
 * Effeitos: escreve somente fórmulas fixas e números observados.
 * Retorno: octetos, zero no silêncio ou SIZE_MAX na recusa.
 * Razão: a estampa precede a mesma sentença, jámais lhe muda o conteúdo.
 */
size_t narrar_observador_de_si(char *destino, size_t capacidade,
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_da_narracao *configuracao,
    uint64_t instante_presente_em_nanossegundos)
{
    enum certeza_da_narracao certeza;
    const char *estampa;
    uint64_t idade;
    int escriptos;

    if (destino == 0 || capacidade == 0 || retrato == 0 || configuracao == 0 ||
        configuracao->modo > MODO_DA_NARRACAO_THEATRAL) return SIZE_MAX;
    destino[0] = '\0';
    if (configuracao->modo == MODO_DA_NARRACAO_SILENCIOSO) return 0;
    certeza = julgar_certeza_do_observador(
        retrato, configuracao, instante_presente_em_nanossegundos);
    estampa = configuracao->modo == MODO_DA_NARRACAO_THEATRAL ? "(@_@) " : "";
    idade = instante_presente_em_nanossegundos >=
            retrato->instante_monotonico_em_nanossegundos ?
        instante_presente_em_nanossegundos -
            retrato->instante_monotonico_em_nanossegundos : UINT64_MAX;
    if (certeza == CERTEZA_IGNORO)
        escriptos = snprintf(destino, capacidade,
            "%s[IGNORO] Meu retrato não governa o presente; idade=%" PRIu64
            " ns.\n", estampa, idade);
    else if (certeza == CERTEZA_INFERI)
        escriptos = snprintf(destino, capacidade,
            "%s[INFERI] Sinto demora: p99=%" PRIu64
            " us alcançou limiar=%" PRIu64 " us.\n", estampa,
            retrato->latencia_p99_em_microssegundos,
            configuracao->p99_alarmante_em_microssegundos);
    else
        escriptos = snprintf(destino, capacidade,
            "%s[OBSERVEI] Concluí %" PRIu64 " operações; erros=%" PRIu64
            ", prazos=%" PRIu64 ".\n", estampa,
            retrato->operacoes_concluidas, retrato->erros,
            retrato->prazos_expirados);
    return escriptos < 0 || (size_t)escriptos >= capacidade ? SIZE_MAX : (size_t)escriptos;
}
