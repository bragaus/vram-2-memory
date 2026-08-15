#ifndef MONITOR_DO_OBSERVATORIO_H
#define MONITOR_DO_OBSERVATORIO_H

#include "retrato_do_observatorio.h"

#include <stddef.h>
#include <stdint.h>

struct configuracao_do_monitor {
    size_t largura_em_colunas;
    int empregar_cor;
};

/*
 * Proposito: converter uma razão finita em régua ASCII.
 * Pre-condições: destino válido, capacidade positiva e largura maior que dois.
 * Effeitos: escreve texto terminado em zero sem exceder a capacidade.
 * Retorno: quantidade escripta, ou zero quando a figura não cabe.
 * Razão: a mesma proporção numérica governa arte larga ou estreita.
 */
size_t desenhar_regua_ascii(char *destino, size_t capacidade,
                            uint64_t parte, uint64_t todo,
                            size_t largura);

/*
 * Proposito: mostrar a marcha recente como linha scintillante ASCII.
 * Pre-condições: amostras e destino válidos; largura positiva.
 * Effeitos: normaliza pelo maior ponto e termina a linha em zero.
 * Retorno: quantidade escripta, ou zero quando não houver espaço.
 * Razão: a ordem das amostras permanece visível sem terminal graphico.
 */
size_t desenhar_linha_scintillante(char *destino, size_t capacidade,
                                   const uint64_t *amostras,
                                   size_t quantidade, size_t largura);

/*
 * Proposito: esculpir um retrato completo em texto consumível por humanos.
 * Pre-condições: retrato, configuração e destino válidos.
 * Effeitos: escreve quadro terminado em zero, com côr apenas se consentida.
 * Retorno: quantidade escripta, ou zero se o quadro não couber.
 * Razão: números e ornamentos derivam juntos da mesma figura immutável.
 */
size_t escrever_quadro_do_observatorio(
    char *destino, size_t capacidade,
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_do_monitor *configuracao);

#endif
