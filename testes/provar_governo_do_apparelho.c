#include "../src/governo_do_apparelho.h"

#include <assert.h>
#include <stdatomic.h>

struct contexto_da_prova_do_governo {
    atomic_int iniciou;
    atomic_int terminar;
};

/*
 * Proposito: representar serviço que vive até receber sua ordem.
 * Pre-condições: configuração válida e contexto vivo. Effeitos: espera activa.
 * Retorno: zero. Razão: a prova não deve depender de CUDA, ublk ou relógio.
 */
int servir_prova_do_governo(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    struct contexto_da_prova_do_governo *prova = contexto;

    assert(configuracao_do_apparelho_e_valida(configuracao));
    atomic_store(&prova->iniciou, 1);
    while (!atomic_load(&prova->terminar)) { }
    return 0;
}

/*
 * Proposito: ordenar o termo do serviço fingido.
 * Pre-condições: contexto vivo. Effeitos: publica marca atomica.
 * Retorno: zero. Razão: a mesma faculdade exterior serve á prova e ao real.
 */
int terminar_prova_do_governo(void *contexto)
{
    struct contexto_da_prova_do_governo *prova = contexto;

    atomic_store(&prova->terminar, 1);
    return 0;
}
