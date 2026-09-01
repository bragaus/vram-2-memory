#ifndef SERVICO_DE_GOVERNO_H
#define SERVICO_DE_GOVERNO_H

#include "governo_do_apparelho.h"

#include <sys/types.h>

/*
 * Proposito: aceitar, julgar, cumprir e responder um único cliente local.
 * Pre-condições: tomada servidora, governo e falha_irrecuperavel vivos.
 * Effeitos: possue uma ligação durante exactamente uma mensagem e resposta;
 *   publica em falha_irrecuperavel se o termo foi interno (accept) ou do cliente.
 * Retorno: zero no percurso ou erro negativo do protocolo ou systema.
 * Razão: só a falha do accept encerra; a falta do cliente é registada e segue.
 */
int atender_cliente_do_governo(int tomada_servidora,
                               struct governo_do_apparelho *governo,
                               int *falha_irrecuperavel);

/*
 * Proposito: conceder audiências até o máximo, ou até a primeira falha
 *   irrecuperável (accept ou argumento); a falta do cliente é registada e segue.
 * Pre-condições: tomada servidora e governo vivos; máximo zero não tem termo.
 * Effeitos: atende clientes em série e regista cada falha no stderr.
 * Retorno: o resultado da última audiência concedida.
 * Razão: o laço proprietário centraliza a conta e o termo das audiências.
 */
int conceder_audiencias_do_governo(int tomada_servidora,
                                   struct governo_do_apparelho *governo,
                                   unsigned int maximo_de_audiencias);

/*
 * Proposito: julgar se o uid de um par local pode governar a instância.
 * Pre-condições: uid do par vindo de SO_PEERCRED; uid esperado é o do dono.
 * Effeitos: nenhum; sentença pura. Retorno: 1 quando aceito, 0 quando recusado.
 * Razão: root ou o próprio dono governam; o modo do grupo deixa de ser porteiro.
 */
int credencial_do_governo_e_aceita(uid_t uid_do_par, uid_t uid_esperado);

#endif
