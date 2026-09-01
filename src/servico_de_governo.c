#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "servico_de_governo.h"
#include "canal_de_governo.h"
#include "ordens_da_instancia.h"

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * THEOREMA DA AUDIENCIA SINGULAR
 * Proposito: receber e responder exactamente uma mensagem de um cliente.
 * Pre-condições: tomada em escuta, governo preparado e falha_irrecuperavel vivo.
 * Effeitos: adquire e restitue ligação e carga transitórias; publica em
 *   falha_irrecuperavel se o termo negativo foi interno (accept) ou do cliente.
 * Retorno: zero no percurso ou primeiro erro negativo.
 * Razão: só a falha do accept é interna; a falta do cliente não encerra o laço.
 */
int atender_cliente_do_governo(int tomada_servidora,
                               struct governo_do_apparelho *governo,
                               int *falha_irrecuperavel)
{
    struct mensagem_de_governo mensagem = {0};
    unsigned char resposta[256];
    uint32_t quantidade = 0;
    int cliente;
    int resultado;

    if (falha_irrecuperavel != 0) *falha_irrecuperavel = 1;
    if (tomada_servidora < 0 || governo == 0) return -EINVAL;
    do {
        cliente = accept4(tomada_servidora, 0, 0, SOCK_CLOEXEC);
    } while (cliente < 0 && errno == EINTR);
    if (cliente < 0) return -errno;
    if (falha_irrecuperavel != 0) *falha_irrecuperavel = 0;
    resultado = receber_mensagem_de_governo(cliente, &mensagem);
    if (resultado == 0) resultado = cumprir_ordem_da_instancia(
        governo, &mensagem, resposta, sizeof(resposta), &quantidade);
    if (resultado == 0) resultado = enviar_mensagem_de_governo(
        cliente, mensagem.cabecalho.operacao, resposta, quantidade);
    destruir_mensagem_de_governo(&mensagem);
    if (close(cliente) != 0 && resultado == 0) resultado = -errno;
    return resultado;
}

/*
 * COROLLARIO DAS AUDIENCIAS SUCCESSIVAS
 * Proposito: conceder audiências até o máximo ou até a primeira negativa.
 * Pre-condições: tomada em escuta e governo vivo; máximo zero não tem termo.
 * Effeitos: regista no stderr cada audiência fallida. Retorno: último resultado.
 * Razão: um só laço proprietário conhece a conta e o termo de toda audiência.
 */
int conceder_audiencias_do_governo(int tomada_servidora,
                                   struct governo_do_apparelho *governo,
                                   unsigned int maximo_de_audiencias)
{
    unsigned int audiencias = 0;
    int resultado;
    int falha_irrecuperavel = 0;

    do {
        resultado = atender_cliente_do_governo(
            tomada_servidora, governo, &falha_irrecuperavel);
        audiencias++;
        if (resultado < 0)
            fprintf(stderr, "A audiência fallou: %d.\n", resultado);
    } while (!falha_irrecuperavel && (maximo_de_audiencias == 0 ||
             audiencias < maximo_de_audiencias));
    return resultado;
}
