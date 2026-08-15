#include "estado_da_requisicao.h"

/*
 * THEOREMA DAS TRANSICOES
 *
 * Proposito: julgar uma aresta do percurso normativo da requisição.
 * Pre-condição: nenhuma; valores alheios à enumeração são recusados.
 * Effeitos: nenhum. Retorno: unidade somente para passagem legítima.
 */
int transicao_da_requisicao_e_valida(enum estado_da_requisicao origem,
                                     enum estado_da_requisicao destino)
{
    /* Cerque-se o domínio, pois inteiro espúrio não deve obter voz. */
    if (origem < ESTADO_DA_REQUISICAO_AGUARDANDO ||
        origem > ESTADO_DA_REQUISICAO_FALHOU ||
        destino < ESTADO_DA_REQUISICAO_AGUARDANDO ||
        destino > ESTADO_DA_REQUISICAO_FALHOU) {
        return 0;
    }

    /* A falha recebe todo estado activo e jámais o abandona. */
    if (destino == ESTADO_DA_REQUISICAO_FALHOU) {
        return origem != ESTADO_DA_REQUISICAO_FALHOU;
    }

    switch (origem) {
    case ESTADO_DA_REQUISICAO_AGUARDANDO:
        return destino == ESTADO_DA_REQUISICAO_TRANSFERINDO ||
               destino == ESTADO_DA_REQUISICAO_CONCLUINDO;
    case ESTADO_DA_REQUISICAO_TRANSFERINDO:
        return destino == ESTADO_DA_REQUISICAO_CONCLUINDO;
    case ESTADO_DA_REQUISICAO_CONCLUINDO:
        return destino == ESTADO_DA_REQUISICAO_AGUARDANDO;
    case ESTADO_DA_REQUISICAO_FALHOU:
        return 0;
    }

    return 0;
}
