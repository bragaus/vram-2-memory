#ifndef ORDEM_DO_CLIENTE_H
#define ORDEM_DO_CLIENTE_H

#include "carga_de_creacao.h"
#include "protocolo_de_governo.h"

struct ordem_do_cliente {
    unsigned int indice;
    uint16_t operacao;
    unsigned char carga[TAMANHO_DA_CARGA_DE_CREACAO];
    uint32_t quantidade_da_carga;
};

/*
 * Proposito: formar create, status ou destroy a partir dos argumentos clientes.
 * Pre-condições: destino vivo; argumentos principiam no nome da operação.
 * Effeitos: publica índice, operação e carga somente no êxito.
 * Retorno: zero ou erro negativo do texto, largura ou domínio.
 * Razão: a entrada main só transporta uma ordem já inteiramente julgada.
 */
int formar_ordem_do_cliente(struct ordem_do_cliente *destino,
                            int quantidade, char *argumentos[]);

#endif
