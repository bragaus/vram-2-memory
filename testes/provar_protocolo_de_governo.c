#include "../src/protocolo_de_governo.h"

#include <errno.h>
#include <string.h>

/*
 * Proposito: demonstrar a ida e volta de uma mensagem canônica.
 * Pre-condições: as funcções publicam o mesmo formato de doze octetos.
 * Effeitos: escreve e lê treze octetos inteiramente locaes.
 * Retorno: unidade na identidade das grandezas, zero na divergência.
 * Razão: a figura válida é a origem contra a qual as recusas serão medidas.
 */
int provar_mensagem_de_governo_valida(void)
{
    unsigned char mensagem[TAMANHO_DO_CABECALHO_DE_GOVERNO + 1U];
    struct cabecalho_de_governo cabecalho = {0};

    memset(mensagem, 0, sizeof(mensagem));
    if (escrever_cabecalho_de_governo(
            mensagem, sizeof(mensagem), OPERACAO_DE_GOVERNO_CONTEMPLAR, 1) < 0)
        return 0;
    mensagem[TAMANHO_DO_CABECALHO_DE_GOVERNO] = 29;
    if (ler_mensagem_de_governo(&cabecalho, mensagem, sizeof(mensagem)) < 0)
        return 0;
    return cabecalho.magia == MAGIA_DO_PROTOCOLO_DE_GOVERNO &&
           cabecalho.versao == VERSAO_DO_PROTOCOLO_DE_GOVERNO &&
           cabecalho.operacao == OPERACAO_DE_GOVERNO_CONTEMPLAR &&
           cabecalho.quantidade_da_carga == 1;
}

/*
 * Proposito: provar que marca, versão e operação estranhas são recusadas.
 * Pre-condições: uma mensagem canônica fornece o exemplar a corromper.
 * Effeitos: altera somente octetos locaes entre as leituras.
 * Retorno: unidade quando cada corrupção recebe seu erro, zero no restante.
 * Razão: nenhuma intenção exterior nasce de vocabulário ou época ignorados.
 */
int provar_identidade_do_protocolo(void)
{
    unsigned char mensagem[TAMANHO_DO_CABECALHO_DE_GOVERNO];
    struct cabecalho_de_governo cabecalho = {0};

    if (escrever_cabecalho_de_governo(
            mensagem, sizeof(mensagem), OPERACAO_DE_GOVERNO_DESTRUIR, 0) < 0)
        return 0;
    mensagem[0] ^= 1U;
    if (ler_mensagem_de_governo(&cabecalho, mensagem, sizeof(mensagem)) !=
        -EPROTO) return 0;
    mensagem[0] ^= 1U;
    mensagem[5] = (unsigned char)(VERSAO_DO_PROTOCOLO_DE_GOVERNO + 1U);
    if (ler_mensagem_de_governo(&cabecalho, mensagem, sizeof(mensagem)) !=
        -EPROTO) return 0;
    mensagem[5] = (unsigned char)VERSAO_DO_PROTOCOLO_DE_GOVERNO;
    mensagem[6] = 0xffU;
    mensagem[7] = 0xffU;
    return ler_mensagem_de_governo(&cabecalho, mensagem, sizeof(mensagem)) ==
           -EOPNOTSUPP;
}
