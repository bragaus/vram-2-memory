#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "servico_de_governo.h"
#include "canal_de_governo.h"
#include "ordens_da_instancia.h"

#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * THEOREMA DA AUDIENCIA SINGULAR
 * Proposito: receber e responder exactamente uma mensagem de um cliente.
 * Pre-condições: tomada em escuta e governo preparado.
 * Effeitos: adquire e restitue ligação e carga transitórias.
 * Retorno: zero no percurso ou primeiro erro negativo.
 * Razão: a fronteira de uma mensagem impede posse indefinida pelo cliente.
 */
int atender_cliente_do_governo(int tomada_servidora,
                               struct governo_do_apparelho *governo)
{
    struct mensagem_de_governo mensagem = {0};
    unsigned char resposta[256];
    uint32_t quantidade = 0;
    int cliente;
    int resultado;

    if (tomada_servidora < 0 || governo == 0) return -EINVAL;
    do {
        cliente = accept4(tomada_servidora, 0, 0, SOCK_CLOEXEC);
    } while (cliente < 0 && errno == EINTR);
    if (cliente < 0) return -errno;
    resultado = receber_mensagem_de_governo(cliente, &mensagem);
    if (resultado == 0) resultado = cumprir_ordem_da_instancia(
        governo, &mensagem, resposta, sizeof(resposta), &quantidade);
    if (resultado == 0) resultado = enviar_mensagem_de_governo(
        cliente, mensagem.cabecalho.operacao, resposta, quantidade);
    destruir_mensagem_de_governo(&mensagem);
    if (close(cliente) != 0 && resultado == 0) resultado = -errno;
    return resultado;
}
