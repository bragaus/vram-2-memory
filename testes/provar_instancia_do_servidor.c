#define _POSIX_C_SOURCE 200809L
#include "../src/instancia_do_servidor.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Proposito: representar o serviço que esta prova não faz nascer.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: abertura da morada independe de CUDA e ublk.
 */
int servir_prova_da_instancia(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)configuracao;
    (void)contexto;
    return 0;
}

/*
 * Proposito: representar o termo que esta prova não precisa exercer.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: o governo exige ambas as faculdades antes da tomada pública.
 */
int terminar_prova_da_instancia(void *contexto)
{
    (void)contexto;
    return 0;
}

/*
 * Proposito: provar modos, exclusividade e restituição da instância inteira.
 * Pre-condições: /tmp acolhe recinto efêmero. Effeitos: crea e remove a prova.
 * Retorno: zero no êxito. Razão: nenhum caminho público ou meio é tocado.
 */
int main(void)
{
    char molde[] = "/tmp/provar-instancia-XXXXXX";
    char raiz[108];
    struct instancia_do_servidor instancia;
    struct instancia_do_servidor homonima;
    struct stat estado;

    assert(mkdtemp(molde) != 0);
    assert(snprintf(raiz, sizeof(raiz), "%s/raiz", molde) > 0);
    assert(abrir_instancia_do_servidor(
        &instancia, raiz, 5, servir_prova_da_instancia,
        terminar_prova_da_instancia, 0) == 0);
    assert(stat(instancia.morada.directorio, &estado) == 0);
    assert((estado.st_mode & 0777) == 0750);
    assert(stat(instancia.morada.processo, &estado) == 0);
    assert((estado.st_mode & 0777) == 0640);
    assert(stat(instancia.morada.tomada, &estado) == 0);
    assert((estado.st_mode & 0777) == 0660);
    assert(abrir_instancia_do_servidor(
        &homonima, raiz, 5, servir_prova_da_instancia,
        terminar_prova_da_instancia, 0) == -EEXIST);
    assert(fechar_instancia_do_servidor(&instancia) == 0);
    assert(access(instancia.morada.directorio, F_OK) != 0 && errno == ENOENT);
    assert(rmdir(raiz) == 0);
    assert(rmdir(molde) == 0);
    return 0;
}
