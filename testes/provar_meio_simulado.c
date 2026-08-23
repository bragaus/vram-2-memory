#include "../src/meio_simulado.h"

#include <errno.h>

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
 * Proposito: provar a cardinalidade do contracto assíncrono simulado.
 * Pre-condições: a taboa publica todas as operações normativas.
 * Effeitos: adquire e restitue um contexto de oito octetos.
 * Retorno: unidade na convergência ou zero na primeira contradicção.
 * Razão: recusa promette zero sentenças; acceitação promette exactamente uma.
 */
int provar_conclusao_assincrona_do_meio(void)
{
    struct configuracao_do_apparelho configuracao = {0};
    struct testemunho_da_conclusao testemunho = {0};
    const struct operacoes_do_meio *operacoes =
        obter_operacoes_do_meio_simulado();
    unsigned char octeto = 7;
    void *contexto = 0;

    configuracao.capacidade_em_bytes = 8;
    configuracao.quantidade_de_filas = 1;
    configuracao.profundidade_das_filas = 2;
    if (operacoes == 0 || operacoes->preparar(&contexto, &configuracao) < 0)
        return 0;
    if (operacoes->escrever(contexto, 0, 0, 0, &octeto, 1,
                            testemunhar_conclusao_do_meio, &testemunho) < 0 ||
        testemunho.quantidade != 0 ||
        operacoes->zerar(contexto, 0, 1, 0, 1,
                         testemunhar_conclusao_do_meio, &testemunho) < 0 ||
        operacoes->escrever(contexto, 0, 0, 0, &octeto, 1,
                            testemunhar_conclusao_do_meio, &testemunho) !=
            -EBUSY ||
        operacoes->colher(contexto, 0, 2) != 2 ||
        testemunho.quantidade != 2 || testemunho.erro != 0 ||
        operacoes->colher(contexto, 0, 1) != 0 ||
        operacoes->ler(contexto, 0, 2, 0, &octeto, 1,
                       testemunhar_conclusao_do_meio, &testemunho) >= 0 ||
        testemunho.quantidade != 2) {
        operacoes->destruir(contexto);
        return 0;
    }
    operacoes->destruir(contexto);
    return 1;
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

    if (!provar_conclusao_assincrona_do_meio()) return 1;
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
