#include "../src/instancia_do_servidor.h"

/*
 * Proposito: representar o serviço que esta prova não faz nascer.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: abertura da morada independe de CUDA e ublk.
 */
int servir_prova_da_instancia(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)configuracao;
    (void)contexto;
    return 0;
}

/*
 * Proposito: representar o termo que esta prova não precisa exercer.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: o governo exige ambas as faculdades antes da tomada pública.
 */
int terminar_prova_da_instancia(void *contexto)
{
    (void)contexto;
    return 0;
}
