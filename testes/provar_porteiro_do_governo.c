#include "../src/servico_de_governo.h"
#include "../src/tomada_do_governo.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Cliente que cala a meio cabeçalho: cortado por tempo, sem prender a audiência.
 */
static void provar_corte_por_tempo(void)
{
    char molde[] = "/tmp/provar-porteiro-XXXXXX";
    char caminho[108];
    struct governo_do_apparelho governo;
    int servidor;
    int cliente;

    memset(&governo, 0, sizeof(governo));
    assert(mkdtemp(molde) != 0);
    assert(snprintf(caminho, sizeof(caminho), "%s/control.sock", molde) > 0);
    servidor = abrir_tomada_servidora_do_governo(caminho);
    assert(servidor >= 0);
    cliente = ligar_tomada_do_governo(caminho);
    assert(cliente >= 0);
    assert(send(cliente, "GG", 2, 0) == 2);
    alarm(10);
    assert(atender_cliente_do_governo(servidor, &governo) == 0);
    alarm(0);
    assert(close(cliente) == 0 && close(servidor) == 0);
    assert(restituir_tomada_do_governo(caminho) == 0);
    assert(rmdir(molde) == 0);
}

/*
 * Reúne o corte por tempo e a política de uid (sentença pura, provável aqui).
 */
int main(void)
{
    assert(credencial_do_governo_e_aceita(0, 1000) == 1);
    assert(credencial_do_governo_e_aceita(1000, 1000) == 1);
    assert(credencial_do_governo_e_aceita(1234, 1000) == 0);
    assert(credencial_do_governo_e_aceita(0, 0) == 1);
    provar_corte_por_tempo();
    return 0;
}
