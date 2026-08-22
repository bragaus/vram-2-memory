#include "../src/ordem_do_cliente.h"

#include <assert.h>
#include <errno.h>

/*
 * Proposito: provar as três ordens, a carga create e operações recusadas.
 * Pre-condições: nenhuma. Effeitos: termina ao primeiro desvio.
 * Retorno: zero no êxito. Razão: a prova antecede toda ligação local.
 */
int main(void)
{
    char *create[] = {
        "create", "7", "65536", "2", "4", "8192", "1000", "1"
    };
    char *status[] = {"status", "7"};
    char *destroy[] = {"destroy", "7"};
    char *incognita[] = {"erase", "7"};
    struct ordem_do_cliente ordem;
    struct configuracao_do_apparelho configuracao;

    assert(formar_ordem_do_cliente(&ordem, 8, create) == 0);
    assert(ordem.indice == 7);
    assert(ordem.operacao == OPERACAO_DE_GOVERNO_CREAR);
    assert(ordem.quantidade_da_carga == TAMANHO_DA_CARGA_DE_CREACAO);
    assert(ler_carga_de_creacao(
        &configuracao, ordem.carga, ordem.quantidade_da_carga) == 0);
    assert(configuracao.indice_da_gpu == 1);
    assert(configuracao.capacidade_em_bytes == UINT64_C(65536));
    assert(formar_ordem_do_cliente(&ordem, 2, status) == 0);
    assert(ordem.operacao == OPERACAO_DE_GOVERNO_CONTEMPLAR);
    assert(ordem.quantidade_da_carga == 0);
    assert(formar_ordem_do_cliente(&ordem, 2, destroy) == 0);
    assert(ordem.operacao == OPERACAO_DE_GOVERNO_DESTRUIR);
    assert(formar_ordem_do_cliente(&ordem, 2, incognita) == -EOPNOTSUPP);
    assert(formar_ordem_do_cliente(&ordem, 1, status) == -EINVAL);
    return 0;
}
