#ifndef SERVIDOR_UBLK_H
#define SERVIDOR_UBLK_H

#include "configuracao.h"

/*
 * Proposito: publicar e servir um dispositivo ublk sobre o meio escolhido.
 * Pre-condições: configuração válida e dependências exteriores disponíveis.
 * Effeitos: cria filas, publica o bloco e serve até ordem de parada.
 * Retorno: zero no termo regular ou erro negativo na primeira falha.
 * Razão: uma só entrada governa acquisição e restituição de todos os recursos.
 */
int executar_servidor_ublk(
    const struct configuracao_do_apparelho *configuracao);

#endif
