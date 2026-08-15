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

/*
 * THEOREMA DA LINHA SCINTILLANTE
 * Proposito: reduzir as amostras mais recentes a oito alturas ASCII.
 * Pre-condições: ponteiros válidos e destino sufficiente.
 * Effeitos: conserva a ordem temporal e termina a escripta em zero.
 * Retorno: quantidade visível, ou zero quando a figura é impossível.
 * Razão: a maior amostra governa a escala sem alterar os números originaes.
 */
size_t desenhar_linha_scintillante(char *destino, size_t capacidade,
                                   const uint64_t *amostras,
                                   size_t quantidade, size_t largura)
{
    static const char alturas[] = " .:-=+*#";
    size_t primeira, visiveis, indice, altura;
    uint64_t maior = 0;

    if (destino == 0 || amostras == 0 || quantidade == 0 || largura == 0)
        return 0;
    visiveis = quantidade < largura ? quantidade : largura;
    if (capacidade <= visiveis) return 0;
    primeira = quantidade - visiveis;
    for (indice = primeira; indice < quantidade; ++indice)
        if (amostras[indice] > maior) maior = amostras[indice];
    for (indice = 0; indice < visiveis; ++indice) {
        if (maior == 0) {
            altura = 0;
        } else {
            altura = (size_t)((long double)amostras[primeira + indice] * 7.0L /
                              (long double)maior);
        }
        destino[indice] = alturas[altura];
    }
    destino[visiveis] = '\0';
    return visiveis;
}
