#ifndef GOVERNO_DO_APPARELHO_H
#define GOVERNO_DO_APPARELHO_H

#include "configuracao.h"

#include <pthread.h>

enum estado_do_governo_do_apparelho {
    ESTADO_DO_GOVERNO_VAZIO = 0,
    ESTADO_DO_GOVERNO_INICIALIZANDO = 1,
    ESTADO_DO_GOVERNO_EM_EXERCICIO = 2,
    ESTADO_DO_GOVERNO_PARANDO = 3,
    ESTADO_DO_GOVERNO_FALHOU = 4
};

typedef int (*servico_do_apparelho)(
    const struct configuracao_do_apparelho *configuracao, void *contexto);
typedef int (*termo_do_apparelho)(void *contexto);

struct governo_do_apparelho {
    pthread_mutex_t exclusao;
    pthread_t fio;
    struct configuracao_do_apparelho configuracao;
    servico_do_apparelho servir;
    termo_do_apparelho ordenar_termo;
    void *contexto;
    enum estado_do_governo_do_apparelho estado;
    int resultado;
    int fio_nascido;
};

/*
 * Proposito: preparar um governo vazio com suas duas faculdades exteriores.
 * Pre-condições: destino vivo e faculdades não nulas. Effeitos: inicia mutex.
 * Retorno: zero ou erro negativo. Razão: nenhuma ordem precede esta fundação.
 */
int preparar_governo_do_apparelho(struct governo_do_apparelho *governo,
                                  servico_do_apparelho servir,
                                  termo_do_apparelho ordenar_termo,
                                  void *contexto);

/*
 * Proposito: fazer nascer em fio próprio o serviço de uma configuração.
 * Pre-condições: governo vazio e configuração válida. Effeitos: inicia fio.
 * Retorno: zero ou erro negativo. Razão: a tomada permanece livre para ordens.
 */
int crear_apparelho_governado(struct governo_do_apparelho *governo,
                              const struct configuracao_do_apparelho *figura);

#endif
