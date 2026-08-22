#include "../src/protocolo_de_governo.h"

#include <errno.h>
#include <stdlib.h>
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

/*
 * Proposito: cercar truncamento, excesso e carga maior que o contracto.
 * Pre-condições: o alocador fornece exactamente a extensão pedida.
 * Effeitos: adquire e restitue exemplares truncados durante a prova.
 * Retorno: unidade quando toda extensão falsa é recusada, zero no restante.
 * Razão: memória exacta permitte ao saneador denunciar leitura indiscreta.
 */
int provar_extensao_da_mensagem(void)
{
    unsigned char mensagem[TAMANHO_DO_CABECALHO_DE_GOVERNO + 2U];
    struct cabecalho_de_governo cabecalho = {0};
    size_t tamanho;

    for (tamanho = 0; tamanho < TAMANHO_DO_CABECALHO_DE_GOVERNO; ++tamanho) {
        size_t reserva = tamanho == 0 ? 1 : tamanho;
        unsigned char *fragmento = malloc(reserva);
        int resultado;

        if (fragmento == 0) return 0;
        memset(fragmento, 0, reserva);
        resultado = ler_mensagem_de_governo(&cabecalho, fragmento, tamanho);
        free(fragmento);
        if (resultado != -EMSGSIZE) return 0;
    }
    if (escrever_cabecalho_de_governo(
            mensagem, sizeof(mensagem), OPERACAO_DE_GOVERNO_CONTEMPLAR, 1) < 0)
        return 0;
    if (ler_mensagem_de_governo(
            &cabecalho, mensagem, TAMANHO_DO_CABECALHO_DE_GOVERNO) !=
            -EMSGSIZE ||
        ler_mensagem_de_governo(&cabecalho, mensagem, sizeof(mensagem)) !=
            -EMSGSIZE) return 0;
    mensagem[8] = 0;
    mensagem[9] = 1;
    mensagem[10] = 0;
    mensagem[11] = 1;
    return ler_mensagem_de_governo(
               &cabecalho, mensagem, TAMANHO_DO_CABECALHO_DE_GOVERNO) ==
           -E2BIG;
}
