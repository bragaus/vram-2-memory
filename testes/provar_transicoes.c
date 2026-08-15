#include "../src/estado_da_requisicao.h"

/*
 * PROVA EXHAUSTIVA DAS TRANSICOES
 * Proposito: confrontar todas as dezasseis passagens e os limites exteriores.
 * Pre-condição: a ordem da enumeração permanece aquella do contracto.
 * Effeitos: nenhum; o retorno zero proclama a demonstração completa.
 */
int main(void)
{
    static const int passagem_esperada[4][4] = {
        {0, 1, 1, 1}, {0, 0, 1, 1},
        {1, 0, 0, 1}, {0, 0, 0, 0}
    };

    for (int origem = 0; origem < 4; origem++) {
        for (int destino = 0; destino < 4; destino++) {
            if (transicao_da_requisicao_e_valida(origem, destino) !=
                passagem_esperada[origem][destino]) {
                return 1;
            }
        }
    }

    if (transicao_da_requisicao_e_valida(-1, 0) ||
        transicao_da_requisicao_e_valida(0, 4)) {
        return 1;
    }

    return 0;
}
