#ifndef NUMERO_DECIMAL_H
#define NUMERO_DECIMAL_H

#include <stdint.h>

/*
 * Proposito: converter texto decimal positivo ou nulo em uint64_t.
 * Pre-condições: texto e destino vivos. Effeitos: publica somente no êxito.
 * Retorno: unidade no êxito e zero no vazio, excesso, sinal ou resto.
 * Razão: cliente e servidor não devem divergir na leitura dos seus números.
 */
int converter_numero_decimal(const char *texto, uint64_t *numero);

#endif
