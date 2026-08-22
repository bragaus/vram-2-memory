#include "governo_do_apparelho.h"
#include "morada_do_governo.h"
#include "numero_decimal.h"
#include "registro_do_governo.h"
#include "servidor_ublk.h"
#include "servico_de_governo.h"
#include "tomada_do_governo.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

struct opcoes_do_servidor {
    const char *raiz;
    unsigned int indice;
    int uma_audiencia;
};

/*
 * Proposito: julgar raiz, modo de prova e índice da entrada servidora.
 * Pre-condições: convenção ordinária de argc e argv.
 * Effeitos: publica opções somente quando a linha inteira é conhecida.
 * Retorno: zero ou erro negativo de texto, aridade ou largura.
 * Razão: nenhuma posse exterior antecede a interpretação completa.
 */
int ler_opcoes_do_servidor(struct opcoes_do_servidor *destino,
                           int quantidade, char *argumentos[])
{
    struct opcoes_do_servidor figura = {RAIZ_ORDINARIA_DO_GOVERNO, 0, 0};
    uint64_t indice;
    int cursor = 1;

    if (destino == 0 || argumentos == 0) return -EINVAL;
    while (cursor < quantidade - 1) {
        if (strcmp(argumentos[cursor], "--root") == 0 &&
            cursor + 1 < quantidade - 1) {
            figura.raiz = argumentos[cursor + 1];
            cursor += 2;
        } else if (strcmp(argumentos[cursor], "--once") == 0) {
            figura.uma_audiencia = 1;
            cursor++;
        } else return -EINVAL;
    }
    if (cursor != quantidade - 1 ||
        !converter_numero_decimal(argumentos[cursor], &indice) ||
        indice > UINT_MAX) return -ERANGE;
    figura.indice = (unsigned int)indice;
    *destino = figura;
    return 0;
}

/*
 * Proposito: exercer no fio proprietário o serviço CUDA integral.
 * Pre-condições: configuração válida. Effeitos: possue CUDA e ublk até termo.
 * Retorno: resultado exacto do servidor. Razão: só vramdiskd conhece esta ponte.
 */
int servir_cuda_governado(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)contexto;
    return executar_servidor_cuda(configuracao);
}

/*
 * Proposito: exercer por chamada ordinária a ordem de termo ublk.
 * Pre-condições: nenhuma. Effeitos: conserva e tenta cumprir a parada.
 * Retorno: zero. Razão: vramdiskctl jámais precisa fabricar signal exterior.
 */
int terminar_cuda_governado(void *contexto)
{
    (void)contexto;
    return ordenar_termo_do_servidor_ublk();
}
