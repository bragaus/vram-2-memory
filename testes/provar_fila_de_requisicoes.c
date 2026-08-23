#include "../src/fila_de_requisicoes.h"

/*
 * PROVA DA ETIQUETA SINGULAR
 * Proposito: confrontar posse, conclusão, restituição e prazo da fila.
 * Pre-condições: a taboa conserva a ordem dos quatro estados públicos.
 * Effeitos: reserva e restitue dois registros durante a experiência.
 * Retorno: zero na convergência e unidade na primeira contradicção.
 * Razão: repetir cada acto revela duplicação que uma passagem só occultaria.
 */
int main(void)
{
    struct fila_de_requisicoes fila = {0};
    struct saude_da_fila primeira = {0}, segunda = {0};
    unsigned char memoria[16] = {0};
    struct registro_da_requisicao *registro;
    unsigned int erro;
    uint32_t etiqueta_vencida = UINT32_MAX;

    for (erro = 0; erro < 7; erro++) {
        if (registrar_resultado_na_saude_da_fila(&primeira, -1, 0) != 0)
            return 1;
    }
    if (registrar_resultado_na_saude_da_fila(&segunda, -1, 0) != 0 ||
        registrar_resultado_na_saude_da_fila(&primeira, 0, 0) != 0 ||
        primeira.erros_consecutivos != 0 ||
        segunda.erros_consecutivos != 1) return 1;
    for (erro = 0; erro < 8; erro++) {
        int terminal = registrar_resultado_na_saude_da_fila(
            &primeira, -1, 0);
        if (terminal != (erro == 7)) return 1;
    }
    if (registrar_resultado_na_saude_da_fila(&primeira, 0, 0) != 1 ||
        registrar_resultado_na_saude_da_fila(&segunda, -1, 1) != 1)
        return 1;

    if (!criar_fila_de_requisicoes(&fila, 2) ||
        criar_fila_de_requisicoes(&fila, 2)) {
        return 1;
    }
    registro = iniciar_requisicao_na_fila(
        &fila, 1, 4096, 16, 1, memoria, 100);
    if (registro == 0 || iniciar_requisicao_na_fila(
            &fila, 1, 4096, 16, 1, memoria, 100) != 0 ||
        contar_requisicoes_transferindo(&fila) != 1) {
        destruir_fila_de_requisicoes(&fila);
        return 1;
    }
    if (!concluir_requisicao_na_fila(&fila, 1, 16) ||
        concluir_requisicao_na_fila(&fila, 1, 16) ||
        contar_requisicoes_transferindo(&fila) != 0 ||
        !rearmar_requisicao_na_fila(&fila, 1) ||
        rearmar_requisicao_na_fila(&fila, 1)) {
        destruir_fila_de_requisicoes(&fila);
        return 1;
    }
    registro = iniciar_requisicao_na_fila(
        &fila, 1, 8192, 16, 0, memoria, 200);
    if (registro == 0 ||
        falhar_requisicao_vencida(&fila, 1, 299, 100, -1) ||
        !falhar_requisicao_vencida(&fila, 1, 300, 100, -1) ||
        concluir_requisicao_na_fila(&fila, 1, 16)) {
        destruir_fila_de_requisicoes(&fila);
        return 1;
    }
    if (iniciar_requisicao_na_fila(
            &fila, 0, 12288, 16, 0, memoria, 400) == 0 ||
        falhar_primeira_requisicao_vencida(
            &fila, 549, 150, -1, &etiqueta_vencida) ||
        !falhar_primeira_requisicao_vencida(
            &fila, 550, 150, -1, &etiqueta_vencida) ||
        etiqueta_vencida != 0) {
        destruir_fila_de_requisicoes(&fila);
        return 1;
    }
    destruir_fila_de_requisicoes(&fila);
    return fila.registros != 0 || fila.profundidade != 0;
}
