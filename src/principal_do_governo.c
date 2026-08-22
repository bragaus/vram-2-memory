#include "canal_de_governo.h"
#include "morada_do_governo.h"
#include "ordem_do_cliente.h"
#include "tomada_do_governo.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Proposito: apresentar as três formas canônicas da entrada cliente.
 * Pre-condições: nome do programma vivo. Effeitos: escreve em stderr.
 * Retorno: nenhum. Razão: entrada antiga recebe recusa e migração explícitas.
 */
void apresentar_uso_do_governo(const char *programma)
{
    fprintf(stderr,
        "Uso:\n"
        "  %s [--root RAIZ] create ID CAP FILAS PROF MAX PRAZO_MS [GPU]\n"
        "  %s [--root RAIZ] status ID\n"
        "  %s [--root RAIZ] destroy ID\n"
        "A entrada antiga directa foi substituída por vramdiskd + vramdiskctl.\n",
        programma, programma, programma);
}

/*
 * THEOREMA DO CLIENTE SEM POSSE
 * Proposito: enviar uma ordem, receber sua resposta e apresentá-la.
 * Pre-condições: raiz e ordem julgadas; servidor em sua tomada.
 * Effeitos: conserva apenas uma ligação transitória e escreve stdout.
 * Retorno: zero no JSON positivo ou erro negativo local ou remoto.
 * Razão: o cliente jámais adquire directório, CUDA ou descritor ublk.
 */
int executar_ordem_do_cliente(const char *raiz,
                              const struct ordem_do_cliente *ordem)
{
    static const unsigned char prefixo_do_exito[] = "{\"ok\":true";
    struct morada_do_governo morada;
    struct mensagem_de_governo resposta = {0};
    int descritor;
    int resultado;

    if (raiz == 0 || ordem == 0) return -EINVAL;
    resultado = formar_morada_do_governo(&morada, raiz, ordem->indice);
    if (resultado < 0) return resultado;
    descritor = ligar_tomada_do_governo(morada.tomada);
    if (descritor < 0) return descritor;
    resultado = enviar_mensagem_de_governo(
        descritor, ordem->operacao, ordem->carga, ordem->quantidade_da_carga);
    if (resultado == 0) resultado = receber_mensagem_de_governo(
        descritor, &resposta);
    (void)close(descritor);
    if (resultado < 0) return resultado;
    if (resposta.cabecalho.operacao != ordem->operacao) resultado = -EPROTO;
    else if (fwrite(resposta.carga, 1,
                    resposta.cabecalho.quantidade_da_carga, stdout) !=
             resposta.cabecalho.quantidade_da_carga || fputc('\n', stdout) < 0)
        resultado = -EIO;
    else if (resposta.cabecalho.quantidade_da_carga <
                 sizeof(prefixo_do_exito) - 1 ||
             memcmp(resposta.carga, prefixo_do_exito,
                    sizeof(prefixo_do_exito) - 1) != 0)
        resultado = -ECANCELED;
    destruir_mensagem_de_governo(&resposta);
    return resultado;
}

/*
 * PROPOSICAO DA ENTRADA CLIENTE
 * Proposito: julgar raiz e argumentos antes de uma única travessia local.
 * Pre-condições: convenção ordinária de argc e argv. Effeitos: apresenta JSON.
 * Retorno: EXIT_SUCCESS no aceite remoto ou EXIT_FAILURE na recusa.
 * Razão: main coordena sem conhecer CUDA, ublk ou governo do fio servidor.
 */
int main(int quantidade_de_argumentos, char *argumentos[])
{
    const char *raiz = RAIZ_ORDINARIA_DO_GOVERNO;
    struct ordem_do_cliente ordem;
    int primeiro = 1;
    int resultado;

    if (quantidade_de_argumentos >= 2 &&
        strcmp(argumentos[1], "--root") == 0) {
        if (quantidade_de_argumentos < 4) {
            apresentar_uso_do_governo(argumentos[0]);
            return EXIT_FAILURE;
        }
        raiz = argumentos[2];
        primeiro = 3;
    }
    resultado = formar_ordem_do_cliente(
        &ordem, quantidade_de_argumentos - primeiro, argumentos + primeiro);
    if (resultado < 0) {
        apresentar_uso_do_governo(argumentos[0]);
        fprintf(stderr, "A ordem cliente foi recusada: %d.\n", resultado);
        return EXIT_FAILURE;
    }
    resultado = executar_ordem_do_cliente(raiz, &ordem);
    if (resultado < 0) {
        fprintf(stderr, "A travessia do governo fallou: %d.\n", resultado);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
