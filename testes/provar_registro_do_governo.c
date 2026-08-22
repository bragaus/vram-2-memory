#define _POSIX_C_SOURCE 200809L
#include "../src/registro_do_governo.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Proposito: provar publicação exclusiva, modo, conteúdo e restituição.
 * Pre-condições: /tmp acolhe um recinto efêmero. Effeitos: crea e remove-o.
 * Retorno: zero no êxito. Razão: a folha real jámais entra nesta prova.
 */
int main(void)
{
    char molde[] = "/tmp/provar-registro-XXXXXX";
    char caminho[4096];
    char recebido[32] = {0};
    char esperado[32];
    struct stat estado;
    int descritor;
    int quantidade;

    assert(mkdtemp(molde) != 0);
    quantidade = snprintf(caminho, sizeof(caminho), "%s/pid", molde);
    assert(quantidade > 0 && (size_t)quantidade < sizeof(caminho));
    assert(publicar_registro_do_governo(caminho, getpid()) == 0);
    assert(stat(caminho, &estado) == 0);
    assert((estado.st_mode & 0777) == 0640);
    assert(publicar_registro_do_governo(caminho, getpid()) == -EEXIST);
    descritor = open(caminho, O_RDONLY | O_CLOEXEC);
    assert(descritor >= 0);
    assert(read(descritor, recebido, sizeof(recebido) - 1) > 0);
    assert(close(descritor) == 0);
    quantidade = snprintf(esperado, sizeof(esperado), "%ld\n", (long)getpid());
    assert(quantidade > 0 && (size_t)quantidade < sizeof(esperado));
    assert(strcmp(recebido, esperado) == 0);
    assert(restituir_registro_do_governo(caminho) == 0);
    assert(restituir_registro_do_governo(caminho) == 0);
    assert(rmdir(molde) == 0);
    return 0;
}
