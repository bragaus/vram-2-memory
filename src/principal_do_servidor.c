#include "governo_do_apparelho.h"
#include "morada_do_governo.h"
#include "numero_decimal.h"
#include "registro_do_governo.h"
#include "servidor_ublk.h"
#include "servico_de_governo.h"
#include "tomada_do_governo.h"

#include <stdio.h>

/*
 * Proposito: exercer no fio proprietário o serviço CUDA integral.
 * Pre-condições: configuração válida. Effeitos: possue CUDA e ublk até termo.
 * Retorno: resultado exacto do servidor. Razão: só vramdiskd conhece esta ponte.
 */
int servir_cuda_governado(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)contexto;
    return executar_servidor_cuda(configuracao);
}

/*
 * Proposito: exercer por chamada ordinária a ordem de termo ublk.
 * Pre-condições: nenhuma. Effeitos: conserva e tenta cumprir a parada.
 * Retorno: zero. Razão: vramdiskctl jámais precisa fabricar signal exterior.
 */
int terminar_cuda_governado(void *contexto)
{
    (void)contexto;
    return ordenar_termo_do_servidor_ublk();
}
