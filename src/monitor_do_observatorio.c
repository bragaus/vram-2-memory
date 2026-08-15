#include "monitor_do_observatorio.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

/*
 * THEOREMA DA REGUA FINITA
 * Proposito: converter uma parte em barra cercada por colchetes.
 * Pre-condições: destino, todo e largura pertencem ao domínio declarado.
 * Effeitos: grava somente caracteres ASCII e o terminador.
 * Retorno: largura no êxito, zero na recusa ou falta de espaço.
 * Razão: limiares accumulados evitam transbordar o producto parte-largura.
 */
size_t desenhar_regua_ascii(char *destino, size_t capacidade,
                            uint64_t parte, uint64_t todo,
                            size_t largura)
{
    size_t interior, indice;
    uint64_t passo, sobra, distribuida = 0, limiar = 0;

    if (destino == 0 || todo == 0 || largura < 3 || capacidade <= largura)
        return 0;
    if (parte > todo) parte = todo;
    interior = largura - 2;
    passo = todo / interior;
    sobra = todo % interior;
    destino[0] = '[';
    for (indice = 0; indice < interior; ++indice) {
        limiar += passo;
        distribuida += sobra;
        if (distribuida >= interior) {
            ++limiar;
            distribuida -= interior;
        }
        destino[indice + 1] = parte != 0 && parte >= limiar ? '#' : '-';
    }
    destino[largura - 1] = ']';
    destino[largura] = '\0';
    return largura;
}

/*
 * THEOREMA DA LINHA SCINTILLANTE
 * Proposito: reduzir as amostras mais recentes a oito alturas ASCII.
 * Pre-condições: ponteiros válidos e destino sufficiente.
 * Effeitos: conserva a ordem temporal e termina a escripta em zero.
 * Retorno: quantidade visível, ou zero quando a figura é impossível.
 * Razão: a maior amostra governa a escala sem alterar os números originaes.
 */
size_t desenhar_linha_scintillante(char *destino, size_t capacidade,
                                   const uint64_t *amostras,
                                   size_t quantidade, size_t largura)
{
    static const char alturas[] = " .:-=+*#";
    size_t primeira, visiveis, indice, altura;
    uint64_t maior = 0;

    if (destino == 0 || amostras == 0 || quantidade == 0 || largura == 0)
        return 0;
    visiveis = quantidade < largura ? quantidade : largura;
    if (capacidade <= visiveis) return 0;
    primeira = quantidade - visiveis;
    for (indice = primeira; indice < quantidade; ++indice)
        if (amostras[indice] > maior) maior = amostras[indice];
    for (indice = 0; indice < visiveis; ++indice) {
        if (maior == 0) {
            altura = 0;
        } else {
            altura = (size_t)((long double)amostras[primeira + indice] * 7.0L /
                              (long double)maior);
        }
        destino[indice] = alturas[altura];
    }
    destino[visiveis] = '\0';
    return visiveis;
}

/*
 * Proposito: converter total de uma janella em razão por segundo.
 * Pre-condições: duração positiva em nanossegundos.
 * Effeitos: nenhum. Retorno: razão saturada no maior inteiro.
 * Razão: quociente e resto evitam multiplicar cegamente uma grandeza vasta.
 */
static uint64_t calcular_por_segundo(uint64_t total,
                                    uint64_t duracao_em_nanossegundos)
{
    long double razao;

    if (duracao_em_nanossegundos == 0) return 0;
    razao = (long double)total * 1000000000.0L /
            (long double)duracao_em_nanossegundos;
    return razao >= (long double)UINT64_MAX ? UINT64_MAX : (uint64_t)razao;
}

/*
 * Proposito: dar número e unidade a um sensor ou declarar ignorância.
 * Pre-condições: destino válido e capacidade positiva.
 * Effeitos: escreve texto cercado e terminado em zero.
 * Retorno: unidade no êxito e zero quando o texto não cabe.
 * Razão: a marca de presença impede que zero se faça passar por medida.
 */
static int escrever_sensor(char *destino, size_t capacidade, int presente,
                           uint32_t valor, const char *unidade)
{
    int escriptos;

    if (destino == 0 || unidade == 0 || capacidade == 0) return 0;
    escriptos = presente ?
        snprintf(destino, capacidade, "%" PRIu32 " %s", valor, unidade) :
        snprintf(destino, capacidade, "IGNORO");
    return escriptos >= 0 && (size_t)escriptos < capacidade;
}
/*
 * THEOREMA DO QUADRO HUMANO
 * Proposito: reunir número, unidade e ornamento num painel finito.
 * Pre-condições: argumentos válidos e retrato já consolidado.
 * Effeitos: grava quadro terminado em zero sem texto exterior.
 * Retorno: octetos visíveis ou zero se a obra não couber.
 * Razão: uma única chamada dá ao quadro época indivisível.
 */
size_t escrever_quadro_do_observatorio(char *destino, size_t capacidade,
    const struct retrato_do_observatorio *retrato,
    const struct configuracao_do_monitor *configuracao)
{
    char regua[129], temperatura[32], potencia[32];
    const char *cor, *fim_da_cor;
    size_t largura; int escriptos;
    uint64_t bytes, vazao, operacoes;
    if (destino == 0 || retrato == 0 || configuracao == 0) return 0;
    largura = configuracao->largura_em_colunas;
    if (largura < 32) largura = 32;
    if (largura > 120) largura = 120;
    bytes = UINT64_MAX - retrato->bytes_lidos < retrato->bytes_escriptos ?
        UINT64_MAX : retrato->bytes_lidos + retrato->bytes_escriptos;
    if (!desenhar_regua_ascii(regua, sizeof(regua), bytes,
            retrato->capacidade_em_bytes == 0 ? 1 : retrato->capacidade_em_bytes,
            largura - 16) ||
        !escrever_sensor(temperatura, sizeof(temperatura),
            retrato->temperatura_da_gpu_presente,
            retrato->temperatura_da_gpu_em_millicelsius, "mC") ||
        !escrever_sensor(potencia, sizeof(potencia),
            retrato->potencia_da_gpu_presente,
            retrato->potencia_da_gpu_em_milliwatts, "mW")) return 0;
    vazao = calcular_por_segundo(bytes,
        retrato->duracao_da_janella_em_nanossegundos);
    operacoes = calcular_por_segundo(retrato->operacoes_concluidas,
        retrato->duracao_da_janella_em_nanossegundos);
    cor = configuracao->empregar_cor ? "\033[36m" : "";
    fim_da_cor = configuracao->empregar_cor ? "\033[0m" : "";
    escriptos = snprintf(destino, capacidade,
        "%s+-- OBSERVATORIO VRAM --+%s\n%s\nVazao: %" PRIu64
        " B/s | Operacoes: %" PRIu64 "/s\nLatencia us: p50=%" PRIu64
        " p95=%" PRIu64 " p99=%" PRIu64 "\nErros: %" PRIu64
        " | Prazos: %" PRIu64 " | Perdidas: %" PRIu64
        "\nGPU: temperatura=%s potencia=%s\n", cor, fim_da_cor, regua, vazao,
        operacoes, retrato->latencia_p50_em_microssegundos,
        retrato->latencia_p95_em_microssegundos,
        retrato->latencia_p99_em_microssegundos, retrato->erros,
        retrato->prazos_expirados, retrato->amostras_perdidas,
        temperatura, potencia);
    return escriptos < 0 || (size_t)escriptos >= capacidade ? 0 : (size_t)escriptos;
}
