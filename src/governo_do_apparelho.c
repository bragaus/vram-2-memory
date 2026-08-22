#include "governo_do_apparelho.h"

#include <errno.h>
#include <string.h>

/*
 * PROPOSICAO DO GOVERNO VAZIO
 * Proposito: fundar a exclusão e as faculdades de um governo sem apparelho.
 * Pre-condições: destino e faculdades vivos. Effeitos: prepara figura vazia.
 * Retorno: zero ou erro negativo do domínio ou de pthread.
 * Razão: toda transição futura depende de uma única exclusão já existente.
 */
int preparar_governo_do_apparelho(struct governo_do_apparelho *governo,
                                  servico_do_apparelho servir,
                                  termo_do_apparelho ordenar_termo,
                                  void *contexto)
{
    int resultado;

    if (governo == 0 || servir == 0 || ordenar_termo == 0) return -EINVAL;
    memset(governo, 0, sizeof(*governo));
    resultado = pthread_mutex_init(&governo->exclusao, 0);
    if (resultado != 0) return -resultado;
    governo->servir = servir;
    governo->ordenar_termo = ordenar_termo;
    governo->contexto = contexto;
    governo->estado = ESTADO_DO_GOVERNO_VAZIO;
    return 0;
}
