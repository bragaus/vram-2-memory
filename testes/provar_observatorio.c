#include "../src/monitor_do_observatorio.h"
#include "../src/observador_de_si.h"

#include <stdint.h>
#include <string.h>
#include <errno.h>

/*
 * Proposito: demonstrar limites, proporção e terminação das figuras breves.
 * Pre-condições: nenhuma; os destinos pertencem á pilha da prova.
 * Effeitos: nenhum fora da pilha. Retorno: unidade ou zero na divergência.
 * Razão: zero, metade e saturação cercam o domínio visual da régua.
 */
static int provar_figuras_breves(void)
{
    const uint64_t amostras[] = {0, 1, 2, 4, 8};
    char figura[32];

    if (desenhar_regua_ascii(figura, sizeof(figura), 0, 8, 10) != 10 ||
        strcmp(figura, "[--------]") != 0) return 0;
    if (desenhar_regua_ascii(figura, sizeof(figura), 4, 8, 10) != 10 ||
        strcmp(figura, "[####----]") != 0) return 0;
    if (desenhar_regua_ascii(figura, sizeof(figura), 9, 8, 10) != 10 ||
        strcmp(figura, "[########]") != 0) return 0;
    if (desenhar_linha_scintillante(
            figura, sizeof(figura), amostras, 5, 3) != 3 ||
        figura[3] != '\0') return 0;
    return desenhar_regua_ascii(figura, 10, 1, 2, 10) == 0;
}

/*
 * Proposito: demonstrar colheita atomica, differença e recusa da regressão.
 * Pre-condições: nenhuma; os dois ábacos principiam zerados.
 * Effeitos: somente atomos locaes são incrementados.
 * Retorno: unidade quando números e cercas coincidem, zero na divergência.
 * Razão: o monitor deve mostrar a janella real e não um accumulo enganoso.
 */
static int provar_retrato_e_regressao(void)
{
    struct contadores_da_fila filas[2] = {0};
    struct retrato_do_observatorio actual, anterior = {0}, janella = {0};

    registrar_operacao_observada(&filas[0], 0, 4096, 1000, 0);
    registrar_operacao_observada(&filas[1], 1, 8192, 5000, -ETIMEDOUT);
    anterior.instante_monotonico_em_nanossegundos = 100;
    if (!colher_retrato_do_observatorio(&actual, filas, 2, 200, 100) ||
        !differenciar_retratos_do_observatorio(
            &janella, &actual, &anterior)) return 0;
    if (janella.bytes_lidos != 4096 || janella.bytes_escriptos != 8192 ||
        janella.operacoes_concluidas != 2 || janella.erros != 1 ||
        janella.prazos_expirados != 1 ||
        janella.duracao_da_janella_em_nanossegundos != 100) return 0;
    anterior = actual;
    actual.bytes_lidos--;
    return !differenciar_retratos_do_observatorio(
        &janella, &actual, &anterior);
}

/*
 * Proposito: demonstrar painel estreito, sem côr e honesto no sensor ausente.
 * Pre-condições: retrato conhecido e destino largo para a prova.
 * Effeitos: escreve somente no destino local.
 * Retorno: unidade se números, ignorância e segurança forem visíveis.
 * Razão: redirecionar a saída não poderá introduzir controle de terminal.
 */
static int provar_quadro_sem_terminal(void)
{
    struct retrato_do_observatorio retrato = {0};
    const struct configuracao_do_monitor configuracao = {20, 0};
    char quadro[1024];
    size_t indice, tamanho;

    retrato.duracao_da_janella_em_nanossegundos = 1000000000ULL;
    retrato.capacidade_em_bytes = 16384;
    retrato.bytes_lidos = 4096;
    retrato.bytes_escriptos = 4096;
    retrato.operacoes_concluidas = 2;
    retrato.latencia_p99_em_microssegundos = 16;
    tamanho = escrever_quadro_do_observatorio(
        quadro, sizeof(quadro), &retrato, &configuracao);
    if (tamanho == 0 || strstr(quadro, "Vazao: 8192 B/s") == 0 ||
        strstr(quadro, "p99=16") == 0 ||
        strstr(quadro, "temperatura=IGNORO") == 0) return 0;
    for (indice = 0; indice < tamanho; ++indice)
        if ((unsigned char)quadro[indice] == 27) return 0;
    return 1;
}

/*
 * Proposito: reunir as demonstrações do observatório num resultado exterior.
 * Pre-condições: nenhuma. Effeitos: nenhum além do código de saída.
 * Retorno: zero se todas as proposições resistem, unidade na primeira queda.
 * Razão: a receita de provas reclama uma porta singular e reproduzível.
 */
int main(void)
{
    return provar_figuras_breves() && provar_retrato_e_regressao() &&
           provar_quadro_sem_terminal() ? 0 : 1;
}
