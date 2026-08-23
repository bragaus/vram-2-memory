#include "../src/meio_cuda.h"

/* Conserva quantas sentenças CUDA chegaram e qual foi a derradeira. */
struct testemunho_da_conclusao_cuda {
    int quantidade;
    int erro;
};

/*
 * Proposito: registrar cada conclusão entregue pela taboa CUDA.
 * Pre-condições: argumento aponta para testemunho vivo.
 * Effeitos: soma uma sentença e conserva seu erro.
 * Retorno: nenhum. Razão: contar torna visível omissão ou duplicação.
 */
void testemunhar_conclusao_cuda(void *argumento, int erro)
{
    struct testemunho_da_conclusao_cuda *testemunho = argumento;

    testemunho->quantidade++;
    testemunho->erro = erro;
}

/*
 * Proposito: provar a promessa differida da taboa CUDA em GPU real.
 * Pre-condições: GPU zero e 4096 octetos de VRAM disponíveis.
 * Effeitos: reserva contexto e memória fixada durante a experiência.
 * Retorno: unidade na convergência ou zero na primeira contradicção.
 * Razão: a execução material deve honrar a mesma cardinalidade do simulador.
 */
int provar_contrato_assincrono_cuda(void)
{
    struct configuracao_do_apparelho configuracao = {0};
    struct testemunho_da_conclusao_cuda testemunho = {0};
    const struct operacoes_do_meio *operacoes = obter_operacoes_do_meio_cuda();
    unsigned char *memoria = 0;
    void *contexto = 0;
    int resultado = 0;

    configuracao.indice_da_gpu = 0;
    configuracao.capacidade_em_bytes = 4096;
    configuracao.quantidade_de_filas = 1;
    if (operacoes == 0 ||
        operacoes->preparar(&contexto, &configuracao) < 0 ||
        operacoes->vincular_fila(contexto, 0) < 0) goto termo;
    memoria = reservar_memoria_intermediaria_cuda(4096);
    if (memoria == 0 ||
        operacoes->aquecer_fila(contexto, 0, memoria, 4096) < 0) goto termo;
    memoria[0] = 29;
    if (operacoes->escrever(contexto, 0, 0, memoria, 1,
                            testemunhar_conclusao_cuda, &testemunho) < 0 ||
        testemunho.quantidade != 0 ||
        operacoes->colher(contexto, 0, 1) != 1 ||
        testemunho.quantidade != 1 || testemunho.erro != 0 ||
        operacoes->colher(contexto, 0, 1) != 0) goto termo;
    resultado = 1;

termo:
    if (!destruir_memoria_intermediaria_cuda(memoria)) resultado = 0;
    if (contexto != 0) operacoes->destruir(contexto);
    return resultado;
}

/*
 * PROVA DA TRAVESSIA CUDA
 * Proposito: confrontar escripta e leitura byte a byte por memória fixada.
 * Pre-condições: GPU de índice zero e 8192 octetos de VRAM disponíveis.
 * Effeitos: reserva VRAM, corrente e dois buffers CPU fixados.
 * Retorno: zero na convergência e unidade na primeira divergência.
 * Razão: padrões distinctos denunciam omissão, troca ou invasão no DMA.
 */
int main(void)
{
    struct meio_cuda meio = {0};
    struct transportador_cuda transportador = {0};
    unsigned char *origem = 0;
    unsigned char *destino = 0;
    int resultado = 1;

    if (!criar_meio_cuda(&meio, 0, 8192) ||
        !criar_transportador_cuda(&transportador, &meio)) goto termo;
    origem = reservar_memoria_intermediaria_cuda(4096);
    destino = reservar_memoria_intermediaria_cuda(4096);
    if (origem == 0 || destino == 0) goto termo;
    for (unsigned int indice = 0; indice < 4096; indice++) {
        origem[indice] = (unsigned char)(indice * 29U + 7U);
        destino[indice] = 0;
    }
    if (!escrever_meio_cuda(&transportador, 2048, origem, 4096) ||
        !ler_meio_cuda(&transportador, 2048, destino, 4096)) goto termo;
    for (unsigned int indice = 0; indice < 4096; indice++) {
        if (origem[indice] != destino[indice]) goto termo;
    }
    if (!zerar_meio_cuda(&transportador, 2048, 4096) ||
        !ler_meio_cuda(&transportador, 2048, destino, 4096)) goto termo;
    for (unsigned int indice = 0; indice < 4096; indice++) {
        if (destino[indice] != 0) goto termo;
    }
    if (ler_meio_cuda(&transportador, 4097, destino, 4096) ||
        escrever_meio_cuda(&transportador, 8192, origem, 1)) goto termo;
    resultado = 0;

termo:
    if (!destruir_memoria_intermediaria_cuda(destino)) resultado = 1;
    if (!destruir_memoria_intermediaria_cuda(origem)) resultado = 1;
    if (!destruir_transportador_cuda(&transportador)) resultado = 1;
    if (!destruir_meio_cuda(&meio)) resultado = 1;
    if (resultado == 0 && !provar_contrato_assincrono_cuda()) resultado = 1;
    return resultado;
}
