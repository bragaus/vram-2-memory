#ifndef ESTADO_DA_REQUISICAO_H
#define ESTADO_DA_REQUISICAO_H

#include <stdint.h>

/*
 * TRACTADO DOS ESTADOS DE UMA REQUISICAO
 *
 * Proposito: fixar as quatro posições legítimas occupadas por uma requisição
 * desde a espera pelo núcleo até a sua conclusão ou ruína.
 *
 * Invariante: uma etiqueta pertence a uma só fila e jámais sustenta duas
 * transferências simultâneas. As transições serão demonstradas n'outro tomo.
 */
enum estado_da_requisicao {
    ESTADO_DA_REQUISICAO_AGUARDANDO,
    ESTADO_DA_REQUISICAO_TRANSFERINDO,
    ESTADO_DA_REQUISICAO_CONCLUINDO,
    ESTADO_DA_REQUISICAO_FALHOU
};

/*
 * REGISTRO MATERIAL DA REQUISICAO
 *
 * Domínio: uma etiqueta fornecida pelo ublk e o espaço de memória que lhe
 * corresponde. Effeito: conservar toda grandeza necessária para que a fila
 * prove o prazo, o resultado e a unicidade da conclusão.
 *
 * Os números possuem largura declarada, pois o apparelho ha-de atravessar
 * machinas diversas sem alterar a medida de seus campos.
 */
struct registro_da_requisicao {
    enum estado_da_requisicao estado;
    void *memoria_intermediaria;
    uint64_t deslocamento;
    uint64_t instante_inicial_em_nanossegundos;
    uint32_t quantidade_de_bytes;
    int resultado;
    uint8_t operacao;
};

/*
 * PROPOSICAO DA PASSAGEM LEGITIMA
 *
 * Domínio: duas posições enumeradas. Contra-domínio: unidade para passagem
 * legítima e zero para prohibição. Effeitos: nenhum registro é transmudado.
 */
int transicao_da_requisicao_e_valida(enum estado_da_requisicao origem,
                                     enum estado_da_requisicao destino);

#endif
