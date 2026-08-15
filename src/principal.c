#include "servidor_ublk.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * LEMMA DO NUMERO DECIMAL
 * Proposito: converter argumento inteiro sem aceitar signal ou resto textual.
 * Pre-condições: texto e destino não nulos.
 * Effeitos: altera o destino somente na conversão integral.
 * Retorno: unidade no êxito e zero no vazio, excesso ou caractere estranho.
 * Razão: strtoull só adquire verdade quando ponteiro final e errno concordam.
 */
int converter_numero_decimal(const char *texto, uint64_t *numero)
{
    unsigned long long valor;
    char *fim;

    if (texto == 0 || numero == 0 || texto[0] == 0 || texto[0] == '-') {
        return 0;
    }
    errno = 0;
    valor = strtoull(texto, &fim, 10);
    if (errno != 0 || *fim != 0) {
        return 0;
    }
    *numero = (uint64_t)valor;
    return 1;
}
