#include "meio_simulado.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Uma fila conserva somente a sentença que ainda não foi colhida. */
struct conclusao_simulada {
    funcao_de_conclusao_do_meio concluir;
    void *argumento;
    int erro;
    int pendente;
};

/* O invólucro reúne o reservatório e uma escrivaninha por fila. */
struct meio_assincrono_simulado {
    struct meio_simulado meio;
    struct conclusao_simulada *conclusoes;
    int quantidade_de_filas;
};

/*
 * LEMMA DO INTERVALLO CONTIDO
 * Proposito: julgar uma região sem sommar grandezas antes do limite.
 * Pre-condições: nenhuma; todo registro estranho será recusado.
 * Effeitos: nenhum. Retorno: unidade somente para região não vazia contida.
 * Razão: subtrahir depois de ordenar impede transbordamento arithmetico.
 */
static int intervallo_do_meio_e_valido(const struct meio_simulado *meio,
                                       uint64_t deslocamento,
                                       uint32_t quantidade_de_bytes)
{
    if (meio == 0 || meio->memoria == 0 || quantidade_de_bytes == 0 ||
        deslocamento > meio->capacidade_em_bytes) {
        return 0;
    }
    return quantidade_de_bytes <= meio->capacidade_em_bytes - deslocamento;
}

/*
 * THEOREMA DA RESERVA ANTECIPADA
 * Proposito: adquirir e zerar toda memória do meio antes de o publicar.
 * Pre-condições: registro não nulo e vazio; capacidade positiva e alojável.
 * Effeitos: estabelece memória e capacidade somente quando logra êxito.
 * Retorno: unidade no êxito e zero sem alterar o registro na falha.
 * Razão: a reserva única aparta o caminho crítico da pressão ulterior.
 */
int criar_meio_simulado(struct meio_simulado *meio,
                        uint64_t capacidade_em_bytes)
{
    unsigned char *memoria;

    if (meio == 0 || meio->memoria != 0 || capacidade_em_bytes == 0 ||
        capacidade_em_bytes > SIZE_MAX) {
        return 0;
    }
    memoria = calloc((size_t)capacidade_em_bytes, 1);
    if (memoria == 0) {
        return 0;
    }
    meio->memoria = memoria;
    meio->capacidade_em_bytes = capacidade_em_bytes;
    return 1;
}

/*
 * COROLLARIO DA RESTITUICAO
 * Proposito: devolver a reserva e reduzir o registro á figura vazia.
 * Pre-condições: nenhuma; o ponteiro nulo é operação sem effeito.
 * Effeitos: liberta a memória possuída e apaga suas grandezas.
 * Retorno: nenhum. Razão: o zero impede uso e libertação repetidos.
 */
void destruir_meio_simulado(struct meio_simulado *meio)
{
    if (meio == 0) {
        return;
    }
    free(meio->memoria);
    meio->memoria = 0;
    meio->capacidade_em_bytes = 0;
}

/*
 * THEOREMA DA LEITURA CONTIDA
 * Proposito: transportar ao destino uma região inteira do reservatório.
 * Pre-condições: meio vivo, destino não nulo e intervallo válido.
 * Effeitos: escreve no destino; o meio permanece immutável.
 * Retorno: unidade no êxito e zero sem copiar na recusa.
 * Razão: o lemma antecede a conversão do deslocamento á medida nativa.
 */
int ler_meio_simulado(const struct meio_simulado *meio, uint64_t deslocamento,
                      void *destino, uint32_t quantidade_de_bytes)
{
    if (destino == 0 || !intervallo_do_meio_e_valido(
            meio, deslocamento, quantidade_de_bytes)) {
        return 0;
    }
    memcpy(destino, meio->memoria + (size_t)deslocamento,
           (size_t)quantidade_de_bytes);
    return 1;
}

/*
 * THEOREMA DA ESCRIPTA CONTIDA
 * Proposito: transportar uma região da origem para o reservatório.
 * Pre-condições: meio vivo, origem não nula e intervallo válido.
 * Effeitos: escreve somente na região consentida do meio.
 * Retorno: unidade no êxito e zero sem copiar na recusa.
 * Razão: leitura e escripta submettem-se á mesma fronteira geométrica.
 */
int escrever_meio_simulado(struct meio_simulado *meio, uint64_t deslocamento,
                           const void *origem,
                           uint32_t quantidade_de_bytes)
{
    if (origem == 0 || !intervallo_do_meio_e_valido(
            meio, deslocamento, quantidade_de_bytes)) {
        return 0;
    }
    memcpy(meio->memoria + (size_t)deslocamento, origem,
           (size_t)quantidade_de_bytes);
    return 1;
}

/*
 * COROLLARIO DA REGIAO NULA
 * Proposito: apagar uma região consentida do reservatório.
 * Pre-condições: meio vivo e intervallo válido.
 * Effeitos: escreve zeros somente na região. Retorno: unidade ou zero.
 * Razão: descarte em meio volátil converge á mesma figura que zero explícito.
 */
int zerar_meio_simulado(struct meio_simulado *meio, uint64_t deslocamento,
                        uint32_t quantidade_de_bytes)
{
    if (!intervallo_do_meio_e_valido(
            meio, deslocamento, quantidade_de_bytes)) {
        return 0;
    }
    memset(meio->memoria + (size_t)deslocamento, 0,
           (size_t)quantidade_de_bytes);
    return 1;
}

/*
 * THEOREMA DA PREPARAÇÃO ASSÍNCRONA
 * Proposito: adquirir reservatório e escrivaninhas antes da publicação.
 * Pre-condições: destino vazio e configuração pertencente ao domínio.
 * Effeitos: publica contexto completo; restitue toda posse na falha.
 * Retorno: zero no êxito, ou erro negativo sem estado parcial.
 * Razão: nenhuma conclusão poderá nascer sem morada previamente numerada.
 */
int preparar_meio_assincrono_simulado(
    void **contexto, const struct configuracao_do_apparelho *configuracao)
{
    struct meio_assincrono_simulado *figura;

    if (contexto == 0 || *contexto != 0 || configuracao == 0 ||
        configuracao->capacidade_em_bytes == 0 ||
        configuracao->quantidade_de_filas <= 0) return -EINVAL;
    figura = calloc(1, sizeof(*figura));
    if (figura == 0) return -ENOMEM;
    if (!criar_meio_simulado(&figura->meio,
                             configuracao->capacidade_em_bytes)) {
        free(figura);
        return -ENOMEM;
    }
    figura->conclusoes = calloc(
        (size_t)configuracao->quantidade_de_filas,
        sizeof(*figura->conclusoes));
    if (figura->conclusoes == 0) {
        destruir_meio_simulado(&figura->meio);
        free(figura);
        return -ENOMEM;
    }
    figura->quantidade_de_filas = configuracao->quantidade_de_filas;
    *contexto = figura;
    return 0;
}

/*
 * COROLLARIO DA RESTITUIÇÃO ASSÍNCRONA
 * Proposito: desfazer em ordem inversa as posses do contexto simulado.
 * Pre-condições: nenhuma conclusão permanece promettida ao consulente.
 * Effeitos: liberta reservatório, escrivaninhas e invólucro.
 * Retorno: nenhum; contexto nulo já representa o termo.
 * Razão: a matéria interior morre antes da figura que a contém.
 */
void destruir_meio_assincrono_simulado(void *contexto)
{
    struct meio_assincrono_simulado *figura = contexto;

    if (figura == 0) return;
    destruir_meio_simulado(&figura->meio);
    free(figura->conclusoes);
    free(figura);
}

/*
 * LEMMA DA FILA SIMULADA
 * Proposito: julgar se uma fila pertence ao contexto preparado.
 * Pre-condições: nenhuma; contexto e índice poderão ser estranhos.
 * Effeitos: nenhum. Retorno: zero no domínio ou -EINVAL na recusa.
 * Razão: a identidade da fila antecede toda conclusão que ella colherá.
 */
int vincular_fila_do_meio_simulado(void *contexto, int indice_da_fila)
{
    const struct meio_assincrono_simulado *figura = contexto;

    if (figura == 0 || indice_da_fila < 0 ||
        indice_da_fila >= figura->quantidade_de_filas) return -EINVAL;
    return 0;
}

/*
 * THEOREMA DO PRIMEIRO PERCURSO
 * Proposito: tocar buffer e reservatório antes que a fila seja publicada.
 * Pre-condições: fila válida e região positiva representável em uint32_t.
 * Effeitos: deposita e recolhe zeros na origem do meio.
 * Retorno: zero no êxito, -EINVAL no domínio ou -EIO no transporte.
 * Razão: o primeiro accesso não deverá sorprender o caminho já exposto.
 */
int aquecer_fila_do_meio_simulado(void *contexto, int indice_da_fila,
                                  void *memoria, size_t quantidade_de_bytes)
{
    struct meio_assincrono_simulado *figura = contexto;
    uint32_t quantidade;

    if (vincular_fila_do_meio_simulado(contexto, indice_da_fila) < 0 ||
        memoria == 0 || quantidade_de_bytes == 0 ||
        quantidade_de_bytes > UINT32_MAX) return -EINVAL;
    quantidade = (uint32_t)quantidade_de_bytes;
    memset(memoria, 0, quantidade_de_bytes);
    if (!escrever_meio_simulado(&figura->meio, 0, memoria, quantidade) ||
        !ler_meio_simulado(&figura->meio, 0, memoria, quantidade)) return -EIO;
    return 0;
}

/*
 * LEMMA DA ESCRIVANINHA NUMERADA
 * Proposito: achar a conclusão pertencente a uma fila válida.
 * Pre-condições: nenhuma; contexto e índice poderão ser estranhos.
 * Effeitos: nenhum. Retorno: escrivaninha ou nulo fora do domínio.
 * Razão: toda consulta de fila emprega uma só demonstração de limites.
 */
static struct conclusao_simulada *achar_conclusao_simulada(
    struct meio_assincrono_simulado *figura, int indice_da_fila)
{
    if (figura == 0 || indice_da_fila < 0 ||
        indice_da_fila >= figura->quantidade_de_filas) return 0;
    return &figura->conclusoes[indice_da_fila];
}

/*
 * THEOREMA DA LEITURA PROMETTIDA
 * Proposito: transportar agora e differir a sentença até a colheita.
 * Pre-condições: fila ociosa, destino vivo e região contida.
 * Effeitos: preenche o destino e arma exactamente uma conclusão.
 * Retorno: zero quando acceita, ou erro negativo sem promessa ulterior.
 * Razão: o simulador imita a separação temporal do futuro DMA.
 */
int submeter_leitura_ao_meio_simulado(
    void *contexto, int indice_da_fila, uint64_t deslocamento, void *destino,
    uint32_t quantidade_de_bytes, funcao_de_conclusao_do_meio concluir,
    void *argumento)
{
    struct meio_assincrono_simulado *figura = contexto;
    struct conclusao_simulada *conclusao = achar_conclusao_simulada(
        figura, indice_da_fila);

    if (conclusao == 0 || concluir == 0 || destino == 0 ||
        !intervallo_do_meio_e_valido(
            &figura->meio, deslocamento, quantidade_de_bytes)) return -EINVAL;
    if (conclusao->pendente) return -EBUSY;
    if (!ler_meio_simulado(&figura->meio, deslocamento, destino,
                           quantidade_de_bytes)) return -EIO;
    conclusao->concluir = concluir;
    conclusao->argumento = argumento;
    conclusao->erro = 0;
    conclusao->pendente = 1;
    return 0;
}

/*
 * THEOREMA DA ESCRIPTA PROMETTIDA
 * Proposito: depositar agora e differir a sentença até a colheita.
 * Pre-condições: fila ociosa, origem viva e região contida.
 * Effeitos: altera o reservatório e arma exactamente uma conclusão.
 * Retorno: zero quando acceita, ou erro negativo sem promessa ulterior.
 * Razão: a mesma ordem temporal governa as duas direcções do transporte.
 */
int submeter_escripta_ao_meio_simulado(
    void *contexto, int indice_da_fila, uint64_t deslocamento,
    const void *origem, uint32_t quantidade_de_bytes,
    funcao_de_conclusao_do_meio concluir, void *argumento)
{
    struct meio_assincrono_simulado *figura = contexto;
    struct conclusao_simulada *conclusao = achar_conclusao_simulada(
        figura, indice_da_fila);

    if (conclusao == 0 || concluir == 0 || origem == 0 ||
        !intervallo_do_meio_e_valido(
            &figura->meio, deslocamento, quantidade_de_bytes)) return -EINVAL;
    if (conclusao->pendente) return -EBUSY;
    if (!escrever_meio_simulado(&figura->meio, deslocamento, origem,
                                quantidade_de_bytes)) return -EIO;
    conclusao->concluir = concluir;
    conclusao->argumento = argumento;
    conclusao->erro = 0;
    conclusao->pendente = 1;
    return 0;
}
