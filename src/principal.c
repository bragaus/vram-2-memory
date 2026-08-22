#include "servidor_ublk.h"
#include "configuracao_decimal.h"

#include <stdio.h>
#include <stdlib.h>

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
    int resultado;

    if (quantidade_de_argumentos < 6 || quantidade_de_argumentos > 7) {
        fprintf(stderr, "Uso: %s capacidade filas profundidade maior_operacao prazo_ms [indice_gpu]\n",
                argumentos[0]);
        return EXIT_FAILURE;
    }
    resultado = ler_configuracao_decimal(
        &configuracao, quantidade_de_argumentos - 1, argumentos + 1);
    if (resultado < 0) {
        fprintf(stderr, "A configuração decimal foi recusada: %d.\n", resultado);
        return EXIT_FAILURE;
    }
    resultado = executar_servidor_cuda(&configuracao);
    if (resultado < 0) {
        fprintf(stderr, "O servidor fallou com o resultado %d.\n", resultado);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
