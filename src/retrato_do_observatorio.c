#include "retrato_do_observatorio.h"

/*
 * Proposito: situar uma demora num degrau logarithmico e finito.
 * Pre-condições: nenhuma; zero pertence ao primeiro degrau.
 * Effeitos: nenhum. Retorno: índice contido no histogramma.
 * Razão: oito ordens binárias bastam ao primeiro painel sem divisão cara.
 */
static size_t achar_degrau_da_latencia(uint64_t nanossegundos)
{
    size_t degrau = 0;
    uint64_t limite = 1000;

    while (degrau + 1 < QUANTIDADE_DE_DEGRAUS_DA_LATENCIA &&
           nanossegundos > limite) {
        limite *= 4;
        ++degrau;
    }
    return degrau;
}

/*
 * Proposito: registrar a conclusão sem tomar trava ou reservar memória.
 * Pre-condições: contadores pertencem á fila chamadora.
 * Effeitos: soma octetos, conclusão, erro e um ponto do histogramma.
 * Retorno: nenhum; entrada nula é recusada sem effeito.
 * Razão: atomos relaxados dão independência ás filas e ao leitor frio.
 */
void registrar_operacao_observada(struct contadores_da_fila *contadores,
                                  int foi_escripta, uint32_t quantidade,
                                  uint64_t latencia_em_nanossegundos,
                                  int resultado)
{
    size_t degrau;

    if (contadores == 0) return;
    degrau = achar_degrau_da_latencia(latencia_em_nanossegundos);
    atomic_fetch_add_explicit(&contadores->operacoes_concluidas, 1,
                              memory_order_relaxed);
    atomic_fetch_add_explicit(&contadores->latencias[degrau], 1,
                              memory_order_relaxed);
    atomic_fetch_add_explicit(foi_escripta ? &contadores->bytes_escriptos :
                              &contadores->bytes_lidos, quantidade,
                              memory_order_relaxed);
    if (resultado < 0)
        atomic_fetch_add_explicit(&contadores->erros, 1, memory_order_relaxed);
}

/*
 * Proposito: approximar um percentil pelo primeiro degrau que o alcança.
 * Pre-condições: histogramma contém oito sommas já consolidadas.
 * Effeitos: nenhum. Retorno: limite superior em microssegundos.
 * Razão: a posição inteira evita ponto fluctuante no observatório basal.
 */
static uint64_t calcular_percentil(const uint64_t *histogramma,
                                  uint64_t total, uint64_t centesimos)
{
    static const uint64_t limites[] = {1, 4, 16, 64, 256, 1024, 4096, 16384};
    uint64_t alvo;
    uint64_t acumulado = 0;
    size_t indice;

    if (histogramma == 0 || total == 0 || centesimos == 0) return 0;
    alvo = total / 100 * centesimos;
    if (total % 100 * centesimos != 0) ++alvo;
    for (indice = 0; indice < QUANTIDADE_DE_DEGRAUS_DA_LATENCIA; ++indice) {
        acumulado += histogramma[indice];
        if (acumulado >= alvo) return limites[indice];
    }
    return limites[QUANTIDADE_DE_DEGRAUS_DA_LATENCIA - 1];
}
