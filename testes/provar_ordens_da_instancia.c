#include "../src/ordens_da_instancia.h"

/*
 * Proposito: representar serviço immediato que a prova não pretende iniciar.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: o ensaio das recusas deve permanecer sem CUDA ou ublk.
 */
int servir_prova_das_ordens(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)configuracao;
    (void)contexto;
    return 0;
}

/*
 * Proposito: representar termo immediato para o governo da prova.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: ambas as faculdades hão de existir antes da primeira mensagem.
 */
int terminar_prova_das_ordens(void *contexto)
{
    (void)contexto;
    return 0;
}
