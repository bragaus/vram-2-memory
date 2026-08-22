#ifndef ORDENS_DA_INSTANCIA_H
#define ORDENS_DA_INSTANCIA_H

#include "canal_de_governo.h"
#include "governo_do_apparelho.h"

#include <stddef.h>

/*
 * Proposito: cumprir uma mensagem julgada e formar sua resposta JSON breve.
 * Pre-condições: governo, mensagem, destino e extensão vivos.
 * Effeitos: executa create, status ou destroy e grava resposta finita.
 * Retorno: zero ou erro negativo do domínio ou do espaço.
 * Razão: uma só fronteira separa protocolo exterior do governo do fio.
 */
int cumprir_ordem_da_instancia(
    struct governo_do_apparelho *governo,
    const struct mensagem_de_governo *mensagem,
    unsigned char *resposta, size_t capacidade, uint32_t *quantidade);

#endif
