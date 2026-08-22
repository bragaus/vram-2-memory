#ifndef SERVICO_DE_GOVERNO_H
#define SERVICO_DE_GOVERNO_H

#include "governo_do_apparelho.h"

/*
 * Proposito: aceitar, julgar, cumprir e responder um único cliente local.
 * Pre-condições: tomada servidora e governo vivos.
 * Effeitos: possue uma ligação durante exactamente uma mensagem e resposta.
 * Retorno: zero no percurso ou erro negativo do protocolo ou systema.
 * Razão: cada accept delimita integralmente a posse transitória do cliente.
 */
int atender_cliente_do_governo(int tomada_servidora,
                               struct governo_do_apparelho *governo);

#endif
