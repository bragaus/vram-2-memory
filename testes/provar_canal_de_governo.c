#include "../src/canal_de_governo.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Proposito: demonstrar transporte integral por duas tomadas locaes ligadas.
 * Pre-condições: o systema fornece socketpair com SOCK_STREAM.
 * Effeitos: abre duas tomadas, transporta carga e restitue todas as posses.
 * Retorno: unidade na identidade e no termo, zero na primeira divergência.
 * Razão: o par local prova o canal sem nome, privilégio ou processo exterior.
 */
int provar_transporte_do_canal_de_governo(void)
{
    static const unsigned char carga[] = {3, 5, 7, 11, 13};
    struct mensagem_de_governo mensagem = {0};
    int tomadas[2] = {-1, -1};
    int resultado = 0;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, tomadas) != 0) goto termo;
    if (enviar_mensagem_de_governo(
            tomadas[0], OPERACAO_DE_GOVERNO_CONTEMPLAR,
            carga, (uint32_t)sizeof(carga)) < 0 ||
        receber_mensagem_de_governo(tomadas[1], &mensagem) < 0) goto termo;
    if (mensagem.cabecalho.operacao != OPERACAO_DE_GOVERNO_CONTEMPLAR ||
        mensagem.cabecalho.quantidade_da_carga != sizeof(carga) ||
        mensagem.carga == 0 ||
        memcmp(mensagem.carga, carga, sizeof(carga)) != 0) goto termo;
    resultado = 1;

termo:
    destruir_mensagem_de_governo(&mensagem);
    destruir_mensagem_de_governo(&mensagem);
    if (mensagem.carga != 0 || mensagem.cabecalho.magia != 0) resultado = 0;
    if (tomadas[0] >= 0) close(tomadas[0]);
    if (tomadas[1] >= 0) close(tomadas[1]);
    return resultado;
}
