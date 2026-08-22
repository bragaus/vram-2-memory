#include "morada_do_governo.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/*
 * THEOREMA DA MORADA SINGULAR
 * Proposito: derivar os três caminhos de uma raiz absoluta e um índice.
 * Pre-condições: destino vivo e raiz sem barra derradeira.
 * Effeitos: publica a figura somente quando todos os textos cabem.
 * Retorno: zero no êxito, -EINVAL no domínio ou -ENAMETOOLONG no excesso.
 * Razão: calcular em figura local prohibe caminhos irmãos divergentes.
 */
int formar_morada_do_governo(struct morada_do_governo *destino,
                             const char *raiz, unsigned int indice)
{
    struct morada_do_governo figura = {{0}, {0}, {0}};
    size_t comprimento;
    int quantidade;

    if (destino == 0 || raiz == 0 || raiz[0] != '/') return -EINVAL;
    comprimento = strlen(raiz);
    if (comprimento == 0 || raiz[comprimento - 1] == '/') return -EINVAL;
    quantidade = snprintf(figura.directorio, sizeof(figura.directorio),
                          "%s/%u", raiz, indice);
    if (quantidade < 0 || (size_t)quantidade >= sizeof(figura.directorio))
        return -ENAMETOOLONG;
    quantidade = snprintf(figura.tomada, sizeof(figura.tomada),
                          "%s/control.sock", figura.directorio);
    if (quantidade < 0 || (size_t)quantidade >= sizeof(figura.tomada))
        return -ENAMETOOLONG;
    quantidade = snprintf(figura.processo, sizeof(figura.processo),
                          "%s/pid", figura.directorio);
    if (quantidade < 0 || (size_t)quantidade >= sizeof(figura.processo))
        return -ENAMETOOLONG;
    *destino = figura;
    return 0;
}
