#include "servidor_ublk.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * LEMMA DO NUMERO DECIMAL
 * Proposito: converter argumento inteiro sem aceitar signal ou resto textual.
 * Pre-condições: texto e destino não nulos.
 * Effeitos: altera o destino somente na conversão integral.
 * Retorno: unidade no êxito e zero no vazio, excesso ou caractere estranho.
 * Razão: strtoull só adquire verdade quando ponteiro final e errno concordam.
 */
int converter_numero_decimal(const char *texto, uint64_t *numero)
{
    unsigned long long valor;
    char *fim;

    if (texto == 0 || numero == 0 || texto[0] == 0 || texto[0] == '-') {
        return 0;
    }
    errno = 0;
    valor = strtoull(texto, &fim, 10);
    if (errno != 0 || *fim != 0) {
        return 0;
    }
    *numero = (uint64_t)valor;
    return 1;
}

/*
 * PROPOSICAO DA ENTRADA PUBLICA
 * Proposito: formar a configuração decimal e entregar o governo ao servidor.
 * Pre-condições: cinco grandezas e, facultativamente, índice da GPU.
 * Effeitos: publica um dispositivo ublk até parada ou falha.
 * Retorno: EXIT_SUCCESS no termo regular e EXIT_FAILURE na recusa.
 * Razão: toda conversão é cercada antes de estreitar sua largura.
 */
int main(int quantidade_de_argumentos, char *argumentos[])
{
    struct configuracao_do_apparelho configuracao = {0};
    uint64_t numeros[6] = {0};
    int indice;
    int resultado;

    if (quantidade_de_argumentos < 6 || quantidade_de_argumentos > 7) {
        fprintf(stderr, "Uso: %s capacidade filas profundidade maior_operacao prazo_ms [indice_gpu]\n",
                argumentos[0]);
        return EXIT_FAILURE;
    }
    for (indice = 1; indice < quantidade_de_argumentos; indice++) {
        if (!converter_numero_decimal(argumentos[indice],
                                      &numeros[indice - 1])) {
            fprintf(stderr, "A grandeza %d não é inteiro decimal.\n", indice);
            return EXIT_FAILURE;
        }
    }
    if (numeros[1] > INT_MAX || numeros[2] > INT_MAX ||
        numeros[3] > UINT32_MAX || numeros[4] > UINT32_MAX ||
        numeros[5] > INT_MAX) {
        fprintf(stderr, "Uma grandeza excede a largura de seu domínio.\n");
        return EXIT_FAILURE;
    }
    configuracao.capacidade_em_bytes = numeros[0];
    configuracao.quantidade_de_filas = (int)numeros[1];
    configuracao.profundidade_das_filas = (int)numeros[2];
    configuracao.maior_operacao_em_bytes = (uint32_t)numeros[3];
    configuracao.prazo_da_operacao_em_milissegundos = (uint32_t)numeros[4];
    configuracao.indice_da_gpu = (int)numeros[5];
    resultado = executar_servidor_cuda(&configuracao);
    if (resultado < 0) {
        fprintf(stderr, "O servidor fallou com o resultado %d.\n", resultado);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
