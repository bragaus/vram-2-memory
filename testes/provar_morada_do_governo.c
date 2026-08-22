#include "../src/morada_do_governo.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

/*
 * Proposito: provar que uma raiz e um índice geram os três nomes canônicos.
 * Pre-condições: nenhuma. Effeitos: termina ao primeiro desvio.
 * Retorno: nenhum. Razão: ambos os processos devem imaginar igual morada.
 */
static void provar_formacao_canonica(void)
{
    struct morada_do_governo morada;

    assert(formar_morada_do_governo(&morada, "/run/vramdisk", 7) == 0);
    assert(strcmp(morada.directorio, "/run/vramdisk/7") == 0);
    assert(strcmp(morada.tomada, "/run/vramdisk/7/control.sock") == 0);
    assert(strcmp(morada.processo, "/run/vramdisk/7/pid") == 0);
}

/*
 * Proposito: provar a recusa de raízes relativas e terminadas em barra.
 * Pre-condições: nenhuma. Effeitos: termina ao primeiro desvio.
 * Retorno: nenhum. Razão: a figura exterior não tolera duas orthographias.
 */
static void provar_raizes_recusadas(void)
{
    struct morada_do_governo morada;

    assert(formar_morada_do_governo(&morada, "run/vramdisk", 0) == -EINVAL);
    assert(formar_morada_do_governo(&morada, "/run/vramdisk/", 0) == -EINVAL);
}

/*
 * Proposito: reunir as provas puras da morada.
 * Pre-condições: nenhuma. Effeitos: termina ao primeiro desvio.
 * Retorno: sempre zero no êxito.
 * Razão: uma entrada mínima faz a prova caber no ritual portátil.
 */
int main(void)
{
    provar_formacao_canonica();
    provar_raizes_recusadas();
    return 0;
}
