#include "../src/configuracao_decimal.h"

#include <assert.h>
#include <errno.h>

/*
 * Proposito: provar cinco campos, GPU facultativa e recusas decimaes.
 * Pre-condições: nenhuma. Effeitos: termina ao primeiro desvio.
 * Retorno: zero no êxito. Razão: ambas as entradas compartilham esta leitura.
 */
int main(void)
{
    char *cinco[] = {"65536", "2", "4", "8192", "1000"};
    char *seis[] = {"65536", "2", "4", "8192", "1000", "3"};
    char *invalida[] = {"65536", "2", "4x", "8192", "1000"};
    struct configuracao_do_apparelho figura;

    assert(ler_configuracao_decimal(&figura, 5, cinco) == 0);
    assert(figura.indice_da_gpu == 0);
    assert(figura.capacidade_em_bytes == UINT64_C(65536));
    assert(figura.quantidade_de_filas == 2);
    assert(figura.profundidade_das_filas == 4);
    assert(figura.maior_operacao_em_bytes == UINT32_C(8192));
    assert(figura.prazo_da_operacao_em_milissegundos == UINT32_C(1000));
    assert(ler_configuracao_decimal(&figura, 6, seis) == 0);
    assert(figura.indice_da_gpu == 3);
    assert(ler_configuracao_decimal(&figura, 5, invalida) == -EINVAL);
    assert(ler_configuracao_decimal(&figura, 4, cinco) == -EINVAL);
    return 0;
}
