#include "fila_de_requisicoes.h"

#include <stdlib.h>

/*
 * THEOREMA DA FILA PREPARADA
 * Proposito: reservar um registro aguardando para cada etiqueta possível.
 * Pre-condições: fila não nula, vazia e profundidade positiva.
 * Effeitos: estabelece a taboa e sua profundidade somente no êxito.
 * Retorno: unidade no êxito e zero sem estado parcial na falha.
 * Razão: calloc prova o producto e faz do zero o estado aguardando.
 */
int criar_fila_de_requisicoes(struct fila_de_requisicoes *fila,
                              uint32_t profundidade)
{
    struct registro_da_requisicao *registros;

    if (fila == 0 || fila->registros != 0 || profundidade == 0) {
        return 0;
    }
    registros = calloc((size_t)profundidade, sizeof(*registros));
    if (registros == 0) {
        return 0;
    }
    fila->registros = registros;
    fila->profundidade = profundidade;
    return 1;
}

/*
 * COROLLARIO DO TERMO DA FILA
 * Proposito: restituir a taboa inteira e apagar sua medida.
 * Pre-condições: nenhuma; fila nula ou vazia não produz effeito.
 * Effeitos: liberta os registros e zera a fila. Retorno: nenhum.
 * Razão: a figura vazia torna innocua uma restituição repetida.
 */
void destruir_fila_de_requisicoes(struct fila_de_requisicoes *fila)
{
    if (fila == 0) {
        return;
    }
    free(fila->registros);
    fila->registros = 0;
    fila->profundidade = 0;
}
