#include "ordens_da_instancia.h"

/*
 * Proposito: converter o estado nativo em vocábulo exterior permanente.
 * Pre-condições: nenhuma. Effeitos: nenhum.
 * Retorno: endereço de texto estático. Razão: números não vazam no JSON.
 */
const char *nome_do_estado_do_governo(
    enum estado_do_governo_do_apparelho estado)
{
    switch (estado) {
    case ESTADO_DO_GOVERNO_VAZIO: return "VAZIO";
    case ESTADO_DO_GOVERNO_INICIALIZANDO: return "INICIALIZANDO";
    case ESTADO_DO_GOVERNO_EM_EXERCICIO: return "EM_EXERCICIO";
    case ESTADO_DO_GOVERNO_PARANDO: return "PARANDO";
    case ESTADO_DO_GOVERNO_FALHOU: return "FALHOU";
    default: return "DESCONHECIDO";
    }
}
