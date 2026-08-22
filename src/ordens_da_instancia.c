#include "ordens_da_instancia.h"
#include "carga_de_creacao.h"
#include <errno.h>
#include <stdio.h>

/*
 * Proposito: converter o estado nativo em vocábulo exterior permanente.
 * Pre-condições: nenhuma. Effeitos: nenhum.
 * Retorno: endereço de texto estático. Razão: números não vazam no JSON.
 */
const char *nome_do_estado_do_governo(
    enum estado_do_governo_do_apparelho estado)
{
    switch (estado) {
    case ESTADO_DO_GOVERNO_VAZIO: return "VAZIO";
    case ESTADO_DO_GOVERNO_INICIALIZANDO: return "INICIALIZANDO";
    case ESTADO_DO_GOVERNO_EM_EXERCICIO: return "EM_EXERCICIO";
    case ESTADO_DO_GOVERNO_PARANDO: return "PARANDO";
    case ESTADO_DO_GOVERNO_FALHOU: return "FALHOU";
    default: return "DESCONHECIDO";
    }
}

/*
 * THEOREMA DA ORDEM GOVERNADA
 * Proposito: cumprir uma operação e formar seu retrato JSON breve.
 * Pre-condições: argumentos vivos e capacidade verdadeira.
 * Effeitos: governa o fio e publica resposta integral.
 * Retorno: zero ou erro negativo de domínio ou espaço da resposta.
 * Razão: erro da ordem pertence ao JSON; erro da fronteira pertence á chamada.
 */
int cumprir_ordem_da_instancia(
    struct governo_do_apparelho *governo,
    const struct mensagem_de_governo *mensagem,
    unsigned char *resposta, size_t capacidade, uint32_t *quantidade)
{
    struct configuracao_do_apparelho configuracao;
    enum estado_do_governo_do_apparelho estado = ESTADO_DO_GOVERNO_VAZIO;
    int resultado_do_servico = 0;
    int erro = 0;
    int tamanho;
    if (governo == 0 || mensagem == 0 || resposta == 0 || quantidade == 0)
        return -EINVAL;
    switch (mensagem->cabecalho.operacao) {
    case OPERACAO_DE_GOVERNO_CREAR:
        erro = ler_carga_de_creacao(&configuracao, mensagem->carga,
                                    mensagem->cabecalho.quantidade_da_carga);
        if (erro == 0) erro = crear_apparelho_governado(governo, &configuracao);
        break;
    case OPERACAO_DE_GOVERNO_CONTEMPLAR:
        if (mensagem->cabecalho.quantidade_da_carga != 0) erro = -EMSGSIZE;
        break;
    case OPERACAO_DE_GOVERNO_DESTRUIR:
        if (mensagem->cabecalho.quantidade_da_carga != 0) erro = -EMSGSIZE;
        else erro = destruir_apparelho_governado(governo);
        break;
    default: erro = -EOPNOTSUPP;
    }
    if (contemplar_apparelho_governado(
            governo, &estado, &resultado_do_servico) < 0 && erro == 0)
        erro = -EINVAL;
    tamanho = snprintf((char *)resposta, capacidade,
        "{\"ok\":%s,\"estado\":\"%s\",\"erro\":%d,\"resultado\":%d}",
        erro == 0 ? "true" : "false", nome_do_estado_do_governo(estado),
        erro, resultado_do_servico);
    if (tamanho < 0 || (size_t)tamanho >= capacidade) return -ENOBUFS;
    *quantidade = (uint32_t)tamanho;
    return 0;
}
