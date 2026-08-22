#include "../src/meio_simulado.h"

/* Conserva quantas sentenças chegaram e qual foi a derradeira. */
struct testemunho_da_conclusao {
    int quantidade;
    int erro;
};

/*
 * Proposito: registrar cada conclusão entregue pelo meio assíncrono.
 * Pre-condições: argumento aponta para testemunho vivo.
 * Effeitos: soma uma sentença e conserva seu erro.
 * Retorno: nenhum. Razão: contar torna visível omissão ou duplicação.
 */
void testemunhar_conclusao_do_meio(void *argumento, int erro)
{
    struct testemunho_da_conclusao *testemunho = argumento;

    testemunho->quantidade++;
    testemunho->erro = erro;
}

/*
 * PROVA DA INTEGRIDADE DO MEIO SIMULADO
 * Proposito: demonstrar reserva, transporte byte a byte e limites.
 * Pre-condições: o contracto do meio conserva as operações declaradas.
 * Effeitos: reserva e restitue memória ordinária durante a experiência.
 * Retorno: zero na convergência e unidade na primeira divergência.
 * Razão: padrões distinctos revelam troca, omissão ou invasão de octetos.
 */
int main(void)
{
    struct meio_simulado meio = {0};
    unsigned char origem[16];
    unsigned char destino[16] = {0};

    for (unsigned int indice = 0; indice < 16; indice++) {
        origem[indice] = (unsigned char)(indice * 17U + 3U);
    }
    if (!criar_meio_simulado(&meio, 32) ||
        criar_meio_simulado(&meio, 32)) {
        return 1;
    }
    if (!escrever_meio_simulado(&meio, 8, origem, 16) ||
        !ler_meio_simulado(&meio, 8, destino, 16)) {
        destruir_meio_simulado(&meio);
        return 1;
    }
    for (unsigned int indice = 0; indice < 16; indice++) {
        if (destino[indice] != origem[indice]) {
            destruir_meio_simulado(&meio);
            return 1;
        }
    }
    if (ler_meio_simulado(&meio, 32, destino, 1) ||
        escrever_meio_simulado(&meio, 17, origem, 16) ||
        ler_meio_simulado(&meio, 0, 0, 1) ||
        escrever_meio_simulado(&meio, 0, 0, 1)) {
        destruir_meio_simulado(&meio);
        return 1;
    }
    destruir_meio_simulado(&meio);
    destruir_meio_simulado(&meio);
    return meio.memoria != 0 || meio.capacidade_em_bytes != 0;
}
