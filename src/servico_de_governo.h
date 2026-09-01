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

/*
 * Proposito: conceder audiências successivas até o máximo ou primeira negativa.
 * Pre-condições: tomada servidora e governo vivos; máximo zero não tem termo.
 * Effeitos: atende clientes em série e regista cada falha no stderr.
 * Retorno: o resultado da última audiência concedida.
 * Razão: o laço proprietário centraliza a conta e o termo das audiências.
 */
int conceder_audiencias_do_governo(int tomada_servidora,
                                   struct governo_do_apparelho *governo,
                                   unsigned int maximo_de_audiencias);

#endif
