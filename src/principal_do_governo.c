#include "canal_de_governo.h"
#include "morada_do_governo.h"
#include "ordem_do_cliente.h"
#include "tomada_do_governo.h"

#include <stdio.h>

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
