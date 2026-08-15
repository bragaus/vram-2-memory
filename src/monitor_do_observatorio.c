#include "monitor_do_observatorio.h"

/*
 * THEOREMA DA REGUA FINITA
 * Proposito: converter uma parte em barra cercada por colchetes.
 * Pre-condições: destino, todo e largura pertencem ao domínio declarado.
 * Effeitos: grava somente caracteres ASCII e o terminador.
 * Retorno: largura no êxito, zero na recusa ou falta de espaço.
 * Razão: limiares accumulados evitam transbordar o producto parte-largura.
 */
size_t desenhar_regua_ascii(char *destino, size_t capacidade,
                            uint64_t parte, uint64_t todo,
                            size_t largura)
{
    size_t interior, indice;
    uint64_t passo, sobra, distribuida = 0, limiar = 0;

    if (destino == 0 || todo == 0 || largura < 3 || capacidade <= largura)
        return 0;
    if (parte > todo) parte = todo;
    interior = largura - 2;
    passo = todo / interior;
    sobra = todo % interior;
    destino[0] = '[';
    for (indice = 0; indice < interior; ++indice) {
        limiar += passo;
        distribuida += sobra;
        if (distribuida >= interior) {
            ++limiar;
            distribuida -= interior;
        }
        destino[indice + 1] = parte != 0 && parte >= limiar ? '#' : '-';
    }
    destino[largura - 1] = ']';
    destino[largura] = '\0';
    return largura;
}
