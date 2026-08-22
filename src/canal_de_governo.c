#include "canal_de_governo.h"

#include <errno.h>
#include <sys/socket.h>

/*
 * Proposito: escrever exactamente a extensão promettida numa tomada.
 * Pre-condições: descritor e memória vivos. Effeitos: avança a tomada.
 * Retorno: zero no êxito ou erro negativo na primeira ruptura.
 * Razão: escriptas parciaes e EINTR não mutilam a mensagem silenciosamente.
 */
static int escrever_todos_os_octetos(int descritor,
                                     const unsigned char *origem,
                                     size_t quantidade)
{
    size_t escriptos = 0;
    while (escriptos < quantidade) {
        ssize_t parcela = send(descritor, origem + escriptos,
                               quantidade - escriptos, MSG_NOSIGNAL);
        if (parcela < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        if (parcela == 0) return -EPIPE;
        escriptos += (size_t)parcela;
    }
    return 0;
}

/*
 * Proposito: enviar cabeçalho e carga sob a ordem canônica do protocolo.
 * Pre-condições: descritor ligado e carga viva quando positiva.
 * Effeitos: escreve duas parcelas sem signal SIGPIPE. Retorno: zero ou erro.
 * Razão: o cabeçalho sempre antecede os octetos cuja extensão declara.
 */
int enviar_mensagem_de_governo(int descritor, uint16_t operacao,
                               const void *carga, uint32_t quantidade)
{
    unsigned char cabecalho[TAMANHO_DO_CABECALHO_DE_GOVERNO];
    int resultado;

    if (quantidade != 0 && carga == 0) return -EINVAL;
    resultado = escrever_cabecalho_de_governo(
        cabecalho, sizeof(cabecalho), operacao, quantidade);
    if (resultado < 0) return resultado;
    resultado = escrever_todos_os_octetos(
        descritor, cabecalho, sizeof(cabecalho));
    if (resultado < 0 || quantidade == 0) return resultado;
    return escrever_todos_os_octetos(descritor, carga, quantidade);
}
