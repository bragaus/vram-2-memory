#include "protocolo_de_governo.h"

/*
 * LEMMA DO VOCABULÁRIO FINITO
 * Proposito: discernir os únicos números publicados como operações.
 * Pre-condições: nenhuma. Effeitos: nenhum.
 * Retorno: unidade no vocabulário e zero para todo número exterior.
 * Razão: intervalos acceitariam por engano lacunas creadas no futuro.
 */
int operacao_de_governo_e_conhecida(uint16_t operacao)
{
    switch (operacao) {
    case OPERACAO_DE_GOVERNO_CREAR:
    case OPERACAO_DE_GOVERNO_CONTEMPLAR:
    case OPERACAO_DE_GOVERNO_DESTRUIR:
    case OPERACAO_DE_GOVERNO_ACTIVAR_SWAP:
    case OPERACAO_DE_GOVERNO_DESACTIVAR_SWAP:
        return 1;
    default:
        return 0;
    }
}
