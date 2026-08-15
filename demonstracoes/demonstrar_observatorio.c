#include "../src/monitor_do_observatorio.h"
#include "../src/observador_de_si.h"
#include "../src/meio_simulado.h"

#include <stdio.h>
#include <string.h>
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

/*
 * Proposito: produzir tráfego real na RAM simulada e colher seus vestígios.
 * Pre-condições: destino válido e vazio.
 * Effeitos: reserva, escreve, lê, compara e restitue um meio de um MiB.
 * Retorno: unidade com retrato completo ou zero depois da limpeza.
 * Razão: números do palco devem nascer de octetos realmente peregrinados.
 */
static int simular_carga(struct retrato_do_observatorio *retrato)
{
    struct meio_simulado meio = {0};
    struct contadores_da_fila contadores = {0};
    const unsigned char origem[16] = "Geometria VRAM!";
    unsigned char destino[16] = {0};
    int resultado = 0;

    if (retrato == 0 || !criar_meio_simulado(&meio, 1048576)) return 0;
    if (!escrever_meio_simulado(&meio, 4096, origem, sizeof(origem))) goto termo;
    registrar_operacao_observada(
        &contadores, 1, sizeof(origem), 4000, 0);
    if (!ler_meio_simulado(&meio, 4096, destino, sizeof(destino)) ||
        memcmp(origem, destino, sizeof(origem)) != 0) goto termo;
    registrar_operacao_observada(
        &contadores, 0, sizeof(destino), 1000, 0);
    if (!colher_retrato_do_observatorio(
            retrato, &contadores, 1, 1000000000ULL, 0)) goto termo;
    retrato->capacidade_em_bytes = meio.capacidade_em_bytes;
    retrato->memoria_do_meio_reservada_em_bytes = meio.capacidade_em_bytes;
    resultado = 1;
termo:
    destruir_meio_simulado(&meio);
    return resultado;
}

int main(void)
{
    struct retrato_do_observatorio retrato = {0};
    struct configuracao_da_narracao voz = {
        MODO_DA_NARRACAO_THEATRAL, 2000000000ULL, 1000
    };

    if (!simular_carga(&retrato)) return 1;
    return apresentar_retrato(&retrato, &voz, 1000000000ULL) ? 0 : 1;
}
