#include "fila_de_requisicoes.h"

#include <errno.h>
#include <stdlib.h>

/*
 * THEOREMA DA OITAVA FALHA
 * Proposito: conservar a série de erros e reconhecer sua ruína terminal.
 * Pre-condições: saúde viva e indicação fatal coherente com erro negativo.
 * Effeitos: zera no êxito, conta no erro e sela a fila ao oitavo ou fatal.
 * Retorno: zero saudável, unidade terminal ou -EINVAL no domínio.
 * Razão: uma sentença victoriosa só interrompe a série da própria fila.
 */
int registrar_resultado_na_saude_da_fila(struct saude_da_fila *saude,
                                         int resultado,
                                         int falha_irrecuperavel)
{
    if (saude == 0 || (falha_irrecuperavel && resultado >= 0)) return -EINVAL;
    if (saude->falha_terminal) return 1;
    if (resultado >= 0) {
        saude->erros_consecutivos = 0;
        return 0;
    }
    if (saude->erros_consecutivos < 8) saude->erros_consecutivos++;
    if (falha_irrecuperavel || saude->erros_consecutivos == 8)
        saude->falha_terminal = 1;
    return saude->falha_terminal;
}

/*
 * THEOREMA DA FILA PREPARADA
 * Proposito: reservar um registro aguardando para cada etiqueta possível.
 * Pre-condições: fila não nula, vazia e profundidade positiva.
 * Effeitos: estabelece a taboa e sua profundidade somente no êxito.
 * Retorno: unidade no êxito e zero sem estado parcial na falha.
 * Razão: calloc prova o producto e faz do zero o estado aguardando.
 */
int criar_fila_de_requisicoes(struct fila_de_requisicoes *fila,
                              uint32_t profundidade)
{
    struct registro_da_requisicao *registros;

    if (fila == 0 || fila->registros != 0 || profundidade == 0) {
        return 0;
    }
    registros = calloc((size_t)profundidade, sizeof(*registros));
    if (registros == 0) {
        return 0;
    }
    fila->registros = registros;
    fila->profundidade = profundidade;
    return 1;
}

/*
 * COROLLARIO DO TERMO DA FILA
 * Proposito: restituir a taboa inteira e apagar sua medida.
 * Pre-condições: nenhuma; fila nula ou vazia não produz effeito.
 * Effeitos: liberta os registros e zera a fila. Retorno: nenhum.
 * Razão: a figura vazia torna innocua uma restituição repetida.
 */
void destruir_fila_de_requisicoes(struct fila_de_requisicoes *fila)
{
    if (fila == 0) {
        return;
    }
    free(fila->registros);
    fila->registros = 0;
    fila->profundidade = 0;
}

/*
 * THEOREMA DA POSSE SINGULAR
 * Proposito: confiar uma etiqueta livre a exactamente uma transferência.
 * Pre-condições: fila viva e etiqueta contida.
 * Effeitos: grava o percurso e muda aguardando para transferindo.
 * Retorno: registro adquirido, ou nulo sem alteração na recusa.
 * Razão: publicar o estado por derradeiro apresenta figura completa.
 */
struct registro_da_requisicao *iniciar_requisicao_na_fila(
    struct fila_de_requisicoes *fila, uint32_t etiqueta,
    uint64_t deslocamento, uint32_t quantidade_de_bytes,
    uint8_t operacao, void *memoria_intermediaria,
    uint64_t instante_inicial_em_nanossegundos)
{
    struct registro_da_requisicao *registro;

    if (fila == 0 || fila->registros == 0 ||
        etiqueta >= fila->profundidade) {
        return 0;
    }
    registro = &fila->registros[etiqueta];
    if (registro->estado != ESTADO_DA_REQUISICAO_AGUARDANDO) {
        return 0;
    }
    registro->memoria_intermediaria = memoria_intermediaria;
    registro->deslocamento = deslocamento;
    registro->instante_inicial_em_nanossegundos =
        instante_inicial_em_nanossegundos;
    registro->quantidade_de_bytes = quantidade_de_bytes;
    registro->etiqueta = etiqueta;
    registro->resultado = 0;
    registro->operacao = operacao;
    registro->estado = ESTADO_DA_REQUISICAO_TRANSFERINDO;
    return registro;
}

/*
 * THEOREMA DA CONCLUSAO UNICA
 * Proposito: publicar um resultado uma vez e restituir sua etiqueta.
 * Pre-condições: fila viva, etiqueta contida e estado transferindo.
 * Effeitos: passa a concluindo e conserva a etiqueta indisponível.
 * Retorno: unidade na primeira conclusão e zero nas demais tentativas.
 * Razão: só a posse activa alcança o umbral da entrega exterior.
 */
int concluir_requisicao_na_fila(struct fila_de_requisicoes *fila,
                                uint32_t etiqueta, int resultado)
{
    struct registro_da_requisicao *registro;

    if (fila == 0 || fila->registros == 0 ||
        etiqueta >= fila->profundidade) {
        return 0;
    }
    registro = &fila->registros[etiqueta];
    if (registro->estado != ESTADO_DA_REQUISICAO_TRANSFERINDO) {
        return 0;
    }
    registro->resultado = resultado;
    registro->estado = ESTADO_DA_REQUISICAO_CONCLUINDO;
    return 1;
}

/*
 * COROLLARIO DA RESTITUICAO DA ETIQUETA
 * Proposito: devolver á espera uma conclusão aceita pelo núcleo.
 * Pre-condições: fila viva, etiqueta contida e registro concluindo.
 * Effeitos: passa a aguardando. Retorno: unidade ou zero na recusa.
 * Razão: a confirmação exterior precede necessariamente a reutilização.
 */
int rearmar_requisicao_na_fila(struct fila_de_requisicoes *fila,
                               uint32_t etiqueta)
{
    if (fila == 0 || fila->registros == 0 ||
        etiqueta >= fila->profundidade ||
        fila->registros[etiqueta].estado !=
            ESTADO_DA_REQUISICAO_CONCLUINDO) {
        return 0;
    }
    fila->registros[etiqueta].estado = ESTADO_DA_REQUISICAO_AGUARDANDO;
    fila->registros[etiqueta].contexto_da_conclusao = 0;
    fila->registros[etiqueta].origem_da_conclusao = 0;
    return 1;
}

/*
 * LEMMA DAS TRANSFERENCIAS PRESENTES
 * Proposito: contar as etiquetas que ainda pertencem ao meio assíncrono.
 * Pre-condições: fila governada exclusivamente pelo fio consulente.
 * Effeitos: nenhum. Retorno: cardinalidade transferindo, ou zero sem fila.
 * Razão: eventos `NOT_READY` reclamam nova consulta, não nova busca exterior.
 */
uint32_t contar_requisicoes_transferindo(
    const struct fila_de_requisicoes *fila)
{
    uint32_t etiqueta;
    uint32_t quantidade = 0;

    if (fila == 0 || fila->registros == 0) return 0;
    for (etiqueta = 0; etiqueta < fila->profundidade; etiqueta++) {
        if (fila->registros[etiqueta].estado ==
            ESTADO_DA_REQUISICAO_TRANSFERINDO) quantidade++;
    }
    return quantidade;
}

/*
 * THEOREMA DO PRAZO FINITO
 * Proposito: levar á falha uma transferência que alcançou seu limite.
 * Pre-condições: instantes monotónicos e prazo positivo.
 * Effeitos: grava resultado e torna a falha terminal.
 * Retorno: unidade no vencimento; zero se inválida ou ainda tempestiva.
 * Razão: compara-se a differença somente após ordenar os instantes.
 */
int falhar_requisicao_vencida(struct fila_de_requisicoes *fila,
                              uint32_t etiqueta, uint64_t instante_actual,
                              uint64_t prazo, int resultado)
{
    struct registro_da_requisicao *registro;

    if (fila == 0 || fila->registros == 0 || prazo == 0 ||
        etiqueta >= fila->profundidade) {
        return 0;
    }
    registro = &fila->registros[etiqueta];
    if (registro->estado != ESTADO_DA_REQUISICAO_TRANSFERINDO ||
        instante_actual < registro->instante_inicial_em_nanossegundos ||
        instante_actual - registro->instante_inicial_em_nanossegundos < prazo) {
        return 0;
    }
    registro->resultado = resultado;
    registro->estado = ESTADO_DA_REQUISICAO_FALHOU;
    return 1;
}
