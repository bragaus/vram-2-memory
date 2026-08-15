#include "observador_de_si.h"

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
