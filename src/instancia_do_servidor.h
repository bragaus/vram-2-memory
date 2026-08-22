#ifndef INSTANCIA_DO_SERVIDOR_H
#define INSTANCIA_DO_SERVIDOR_H

#include "governo_do_apparelho.h"
#include "morada_do_governo.h"

struct instancia_do_servidor {
    struct morada_do_governo morada;
    struct governo_do_apparelho governo;
    int tomada_servidora;
    int governo_preparado;
    int registro_publicado;
    int recinto_preparado;
};

/*
 * Proposito: adquirir recinto, registro, tomada e governo de uma instância.
 * Pre-condições: raiz canônica, índice e faculdades válidos.
 * Effeitos: publica todas as posses ou restitue a successão parcial.
 * Retorno: zero ou primeiro erro negativo.
 * Razão: vramdiskd só escuta depois de sua morada inteira existir.
 */
int abrir_instancia_do_servidor(
    struct instancia_do_servidor *instancia, const char *raiz,
    unsigned int indice, servico_do_apparelho servir,
    termo_do_apparelho terminar, void *contexto);

/*
 * Proposito: restituir governo, tomada, registro e recinto em ordem inversa.
 * Pre-condições: figura aberta total ou parcialmente por esta execução.
 * Effeitos: encerra todas as posses conhecidas.
 * Retorno: zero ou primeiro erro negativo sem omittir limpeza ulterior.
 * Razão: cada marca distingue exactamente aquilo que se pode remover.
 */
int fechar_instancia_do_servidor(struct instancia_do_servidor *instancia);

#endif
