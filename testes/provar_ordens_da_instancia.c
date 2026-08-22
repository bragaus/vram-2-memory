#include "../src/ordens_da_instancia.h"

#include <assert.h>
#include <string.h>

/*
 * Proposito: representar serviço immediato que a prova não pretende iniciar.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: o ensaio das recusas deve permanecer sem CUDA ou ublk.
 */
int servir_prova_das_ordens(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)configuracao;
    (void)contexto;
    return 0;
}

/*
 * Proposito: representar termo immediato para o governo da prova.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: ambas as faculdades hão de existir antes da primeira mensagem.
 */
int terminar_prova_das_ordens(void *contexto)
{
    (void)contexto;
    return 0;
}

/*
 * Proposito: provar status vazio e recusas exteriores convertidas em JSON.
 * Pre-condições: pthread disponível. Effeitos: funda e restitue um governo.
 * Retorno: zero no êxito. Razão: erros da ordem jámais rompem sua fronteira.
 */
int main(void)
{
    struct governo_do_apparelho governo;
    struct mensagem_de_governo mensagem = {0};
    unsigned char resposta[160];
    uint32_t quantidade;

    assert(preparar_governo_do_apparelho(
        &governo, servir_prova_das_ordens,
        terminar_prova_das_ordens, 0) == 0);
    mensagem.cabecalho.operacao = OPERACAO_DE_GOVERNO_CONTEMPLAR;
    assert(cumprir_ordem_da_instancia(
        &governo, &mensagem, resposta, sizeof(resposta), &quantidade) == 0);
    assert(quantidade > 0 && strstr((char *)resposta, "\"ok\":true") != 0);
    assert(strstr((char *)resposta, "\"estado\":\"VAZIO\"") != 0);
    mensagem.cabecalho.quantidade_da_carga = 1;
    assert(cumprir_ordem_da_instancia(
        &governo, &mensagem, resposta, sizeof(resposta), &quantidade) == 0);
    assert(strstr((char *)resposta, "\"ok\":false") != 0);
    mensagem.cabecalho.operacao = OPERACAO_DE_GOVERNO_CREAR;
    mensagem.cabecalho.quantidade_da_carga = 0;
    assert(cumprir_ordem_da_instancia(
        &governo, &mensagem, resposta, sizeof(resposta), &quantidade) == 0);
    assert(strstr((char *)resposta, "\"ok\":false") != 0);
    assert(encerrar_governo_do_apparelho(&governo) == 0);
    return 0;
}
