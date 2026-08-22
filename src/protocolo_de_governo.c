#include "protocolo_de_governo.h"

#include <errno.h>

/*
 * LEMMA DO VOCABULÁRIO FINITO
 * Proposito: discernir os únicos números publicados como operações.
 * Pre-condições: nenhuma. Effeitos: nenhum.
 * Retorno: unidade no vocabulário e zero para todo número exterior.
 * Razão: intervalos acceitariam por engano lacunas creadas no futuro.
 */
int operacao_de_governo_e_conhecida(uint16_t operacao)
{
    switch (operacao) {
    case OPERACAO_DE_GOVERNO_CREAR:
    case OPERACAO_DE_GOVERNO_CONTEMPLAR:
    case OPERACAO_DE_GOVERNO_DESTRUIR:
    case OPERACAO_DE_GOVERNO_ACTIVAR_SWAP:
    case OPERACAO_DE_GOVERNO_DESACTIVAR_SWAP:
        return 1;
    default:
        return 0;
    }
}

/*
 * THEOREMA DA ESCRIPTA EXTERIOR
 * Proposito: converter as grandezas canônicas em doze octetos ordenados.
 * Pre-condições: destino declara sua capacidade verdadeira.
 * Effeitos: grava o cabeçalho inteiro ou conserva o destino intacto.
 * Retorno: zero no êxito, -EINVAL no domínio ou -ENOBUFS no espaço.
 * Razão: a ordem maior primeiro jámais depende da representação nativa.
 */
int escrever_cabecalho_de_governo(unsigned char *destino, size_t capacidade,
                                  uint16_t operacao, uint32_t carga)
{
    if (destino == 0 || !operacao_de_governo_e_conhecida(operacao) ||
        carga > LIMITE_DA_CARGA_DE_GOVERNO) return -EINVAL;
    if (capacidade < TAMANHO_DO_CABECALHO_DE_GOVERNO) return -ENOBUFS;
    destino[0] = (unsigned char)(MAGIA_DO_PROTOCOLO_DE_GOVERNO >> 24);
    destino[1] = (unsigned char)(MAGIA_DO_PROTOCOLO_DE_GOVERNO >> 16);
    destino[2] = (unsigned char)(MAGIA_DO_PROTOCOLO_DE_GOVERNO >> 8);
    destino[3] = (unsigned char)MAGIA_DO_PROTOCOLO_DE_GOVERNO;
    destino[4] = (unsigned char)(VERSAO_DO_PROTOCOLO_DE_GOVERNO >> 8);
    destino[5] = (unsigned char)VERSAO_DO_PROTOCOLO_DE_GOVERNO;
    destino[6] = (unsigned char)(operacao >> 8);
    destino[7] = (unsigned char)operacao;
    destino[8] = (unsigned char)(carga >> 24);
    destino[9] = (unsigned char)(carga >> 16);
    destino[10] = (unsigned char)(carga >> 8);
    destino[11] = (unsigned char)carga;
    return 0;
}
