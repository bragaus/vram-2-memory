#ifndef CONFIGURACAO_DECIMAL_H
#define CONFIGURACAO_DECIMAL_H

#include "configuracao.h"

/*
 * Proposito: formar configuração de cinco ou seis textos decimaes.
 * Pre-condições: destino vivo; argumentos principiam na capacidade.
 * Effeitos: publica figura somente depois de conversão, largura e domínio.
 * Retorno: zero no êxito ou erro negativo sem figura parcial.
 * Razão: entrada antiga e create hão de estreitar números igualmente.
 */
int ler_configuracao_decimal(struct configuracao_do_apparelho *destino,
                             int quantidade, char *argumentos[]);

#endif
