#include "../src/monitor_do_observatorio.h"
#include "../src/observador_de_si.h"

#include <stdio.h>
#include <unistd.h>

/*
 * Proposito: apresentar um retrato e sua voz pela mesma época immutável.
 * Pre-condições: retrato e configuração já julgados pelo chamador.
 * Effeitos: escreve quadro e sentença em stdout.
 * Retorno: unidade no êxito e zero na recusa ou escripta incompleta.
 * Razão: o palco não poderá separar o ornamento de sua fonte numérica.
 */
static int apresentar_retrato(
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_da_narracao *configuracao,
    uint64_t instante_presente)
{
    const struct configuracao_do_monitor monitor = {
        72, isatty(STDOUT_FILENO)
    };
    char quadro[2048], voz[256];
    size_t tamanho_do_quadro, tamanho_da_voz;

    tamanho_do_quadro = escrever_quadro_do_observatorio(
        quadro, sizeof(quadro), retrato, &monitor);
    tamanho_da_voz = narrar_observador_de_si(
        voz, sizeof(voz), retrato, configuracao, instante_presente);
    if (tamanho_do_quadro == 0 || tamanho_da_voz == SIZE_MAX) return 0;
    return fwrite(quadro, 1, tamanho_do_quadro, stdout) == tamanho_do_quadro &&
           fwrite(voz, 1, tamanho_da_voz, stdout) == tamanho_da_voz;
}

int main(void)
{
    struct retrato_do_observatorio retrato = {0};
    struct configuracao_da_narracao voz = {
        MODO_DA_NARRACAO_THEATRAL, 2000000000ULL, 1000
    };

    retrato.instante_monotonico_em_nanossegundos = 1000000000ULL;
    retrato.duracao_da_janella_em_nanossegundos = 1000000000ULL;
    retrato.capacidade_em_bytes = 1048576;
    retrato.memoria_do_meio_reservada_em_bytes = 524288;
    retrato.bytes_lidos = 4096;
    retrato.operacoes_concluidas = 1;
    retrato.latencia_p99_em_microssegundos = 16;
    return apresentar_retrato(&retrato, &voz, 1000000000ULL) ? 0 : 1;
}
