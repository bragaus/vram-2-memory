#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "servico_de_governo.h"
#include "canal_de_governo.h"
#include "ordens_da_instancia.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef PRAZO_DA_AUDIENCIA_EM_SEGUNDOS
#define PRAZO_DA_AUDIENCIA_EM_SEGUNDOS 5
#endif

/*
 * Proposito: atar prazo finito de recepção e envio á audiência de um cliente.
 * Pre-condições: descritor de cliente ligado. Effeitos: liga SO_RCVTIMEO e SO_SNDTIMEO.
 * Retorno: zero no êxito ou erro negativo do systema.
 * Razão: cliente que cala a meio caminho é cortado em tempo finito (slowloris).
 */
static int atar_prazos_da_audiencia(int cliente)
{
    struct timeval prazo = { .tv_sec = PRAZO_DA_AUDIENCIA_EM_SEGUNDOS, .tv_usec = 0 };
    if (setsockopt(cliente, SOL_SOCKET, SO_RCVTIMEO, &prazo, sizeof(prazo)) != 0)
        return -errno;
    if (setsockopt(cliente, SOL_SOCKET, SO_SNDTIMEO, &prazo, sizeof(prazo)) != 0)
        return -errno;
    return 0;
}

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
    resultado = atar_prazos_da_audiencia(cliente);
    if (resultado == 0)
        resultado = receber_mensagem_de_governo(cliente, &mensagem);
    if (resultado == 0) resultado = cumprir_ordem_da_instancia(
        governo, &mensagem, resposta, sizeof(resposta), &quantidade);
    if (resultado == 0) resultado = enviar_mensagem_de_governo(
        cliente, mensagem.cabecalho.operacao, resposta, quantidade);
    if (resultado == -EAGAIN || resultado == -EWOULDBLOCK) resultado = 0;
    destruir_mensagem_de_governo(&mensagem);
    if (close(cliente) != 0 && resultado == 0) resultado = -errno;
    return resultado;
}
