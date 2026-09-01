#ifndef SERVICO_DE_GOVERNO_H
#define SERVICO_DE_GOVERNO_H

#include "governo_do_apparelho.h"

#include <sys/types.h>

/*
 * Proposito: aceitar, julgar, cumprir e responder um único cliente local.
 * Pre-condições: tomada servidora e governo vivos.
 * Effeitos: possue uma ligação durante exactamente uma mensagem e resposta.
 * Retorno: zero no percurso ou erro negativo do protocolo ou systema.
 * Razão: cada accept delimita integralmente a posse transitória do cliente.
 */
int atender_cliente_do_governo(int tomada_servidora,
                               struct governo_do_apparelho *governo);

/*
 * Proposito: julgar se o uid de um par local pode governar a instância.
 * Pre-condições: uid do par vindo de SO_PEERCRED; uid esperado é o do dono.
 * Effeitos: nenhum; sentença pura. Retorno: 1 quando aceito, 0 quando recusado.
 * Razão: root ou o próprio dono governam; o modo do grupo deixa de ser porteiro.
 */
int credencial_do_governo_e_aceita(uid_t uid_do_par, uid_t uid_esperado);

#endif
