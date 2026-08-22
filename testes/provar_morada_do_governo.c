#define _POSIX_C_SOURCE 200809L
#include "../src/morada_do_governo.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
 * Proposito: provar exclusividade e modo do recinto em raiz temporária.
 * Pre-condições: /tmp permite crear directório. Effeitos: crea e remove prova.
 * Retorno: nenhum. Razão: a prova não deve tocar a morada real do serviço.
 */
static void provar_recinto_exclusivo(void)
{
    char molde[] = "/tmp/provar-morada-XXXXXX";
    char raiz[LIMITE_DO_CAMINHO_DO_GOVERNO];
    struct morada_do_governo morada;
    struct stat estado;
    int quantidade;

    assert(mkdtemp(molde) != 0);
    quantidade = snprintf(raiz, sizeof(raiz), "%s/raiz", molde);
    assert(quantidade > 0 && (size_t)quantidade < sizeof(raiz));
    assert(formar_morada_do_governo(&morada, raiz, 3) == 0);
    assert(preparar_morada_do_governo(&morada, raiz) == 0);
    assert(stat(morada.directorio, &estado) == 0);
    assert((estado.st_mode & 0777) == 0750);
    assert(preparar_morada_do_governo(&morada, raiz) == -EEXIST);
    assert(rmdir(morada.directorio) == 0);
    assert(rmdir(raiz) == 0);
    assert(rmdir(molde) == 0);
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
    provar_recinto_exclusivo();
    return 0;
}
