#include "meio_cuda.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Uma fila CUDA conserva a sentença que aguarda colheita. */
struct conclusao_cuda {
    funcao_de_conclusao_do_meio concluir;
    void *argumento;
    int erro;
    int pendente;
};

/* O invólucro reúne a VRAM, seus transportadores e sentenças. */
struct meio_assincrono_cuda {
    struct meio_cuda meio;
    struct transportador_cuda *transportadores;
    struct conclusao_cuda *conclusoes;
    int quantidade_de_filas;
};

/*
 * THEOREMA DA RESERVA NA GPU
 * Proposito: escolher o engenho, adquirir VRAM e zerar toda a extensão.
 * Pre-condições: meio vazio, índice não negativo e capacidade alojável.
 * Effeitos: altera o dispositivo CUDA corrente e estabelece a reserva.
 * Retorno: unidade no êxito; zero com libertação na primeira falha.
 * Razão: o zero inicial impede que leitura preceda escripta com lixo antigo.
 */
int criar_meio_cuda(struct meio_cuda *meio, int indice_da_gpu,
                    uint64_t capacidade_em_bytes)
{
    unsigned char *memoria;

    if (meio == 0 || meio->memoria_da_gpu != 0 || indice_da_gpu < 0 ||
        capacidade_em_bytes == 0 || capacidade_em_bytes > SIZE_MAX) {
        return 0;
    }
    if (cudaSetDevice(indice_da_gpu) != cudaSuccess ||
        cudaMalloc((void **)&memoria, (size_t)capacidade_em_bytes) !=
            cudaSuccess) {
        return 0;
    }
    meio->memoria_da_gpu = memoria;
    meio->capacidade_em_bytes = capacidade_em_bytes;
    meio->indice_da_gpu = indice_da_gpu;
    if (cudaMemset(memoria, 0, (size_t)capacidade_em_bytes) != cudaSuccess) {
        destruir_meio_cuda(meio);
        return 0;
    }
    return 1;
}

/*
 * COROLLARIO DA RESTITUICAO DA VRAM
 * Proposito: devolver ao engenho exacto a reserva que elle concedeu.
 * Pre-condições: nenhuma; meio nulo ou vazio é termo regular.
 * Effeitos: escolhe a GPU, liberta VRAM e zera o registro.
 * Retorno: unidade no termo ou zero se CUDA conservar a posse.
 * Razão: o registro só se apaga depois da confirmação de cudaFree.
 */
int destruir_meio_cuda(struct meio_cuda *meio)
{
    if (meio == 0 || meio->memoria_da_gpu == 0) {
        return 1;
    }
    if (cudaSetDevice(meio->indice_da_gpu) != cudaSuccess ||
        cudaFree(meio->memoria_da_gpu) != cudaSuccess) {
        return 0;
    }
    meio->memoria_da_gpu = 0;
    meio->capacidade_em_bytes = 0;
    meio->indice_da_gpu = 0;
    return 1;
}

/*
 * THEOREMA DA CORRENTE EXCLUSIVA
 * Proposito: preparar uma corrente não bloqueante para uma só fila.
 * Pre-condições: transportador vazio e meio vivo.
 * Effeitos: escolhe a GPU e adquire cudaStream_t.
 * Retorno: unidade no êxito ou zero sem estado parcial.
 * Razão: a corrente ordinária introduziria dependências entre filas irmãs.
 */
int criar_transportador_cuda(struct transportador_cuda *transportador,
                             struct meio_cuda *meio)
{
    cudaStream_t corrente;

    if (transportador == 0 || transportador->corrente != 0 || meio == 0 ||
        meio->memoria_da_gpu == 0) {
        return 0;
    }
    if (cudaSetDevice(meio->indice_da_gpu) != cudaSuccess ||
        cudaStreamCreateWithFlags(&corrente, cudaStreamNonBlocking) !=
            cudaSuccess) {
        return 0;
    }
    transportador->meio = meio;
    transportador->corrente = corrente;
    return 1;
}

/*
 * COROLLARIO DO TERMO DA CORRENTE
 * Proposito: esperar toda cópia e restituir a corrente á GPU.
 * Pre-condições: nenhuma; transportador vazio é termo regular.
 * Effeitos: synchroniza, destrói e zera o transportador.
 * Retorno: unidade somente quando synchronização e destruição concordam.
 * Razão: mesmo erro tardio não dispensa a tentativa de restituir a posse.
 */
int destruir_transportador_cuda(struct transportador_cuda *transportador)
{
    cudaError_t resultado_da_synchronizacao;
    cudaError_t resultado_da_destruicao;

    if (transportador == 0 || transportador->corrente == 0) {
        return 1;
    }
    if (transportador->meio == 0 ||
        cudaSetDevice(transportador->meio->indice_da_gpu) != cudaSuccess) {
        return 0;
    }
    resultado_da_synchronizacao =
        cudaStreamSynchronize(transportador->corrente);
    resultado_da_destruicao = cudaStreamDestroy(transportador->corrente);
    if (resultado_da_destruicao == cudaSuccess) {
        transportador->corrente = 0;
        transportador->meio = 0;
    }
    return resultado_da_synchronizacao == cudaSuccess &&
           resultado_da_destruicao == cudaSuccess;
}

/*
 * THEOREMA DA MEMORIA INTERMEDIARIA FIXA
 * Proposito: adquirir na CPU região que DMA possa alcançar directamente.
 * Pre-condições: quantidade positiva.
 * Effeitos: chama cudaHostAlloc com visibilidade entre contextos.
 * Retorno: endereço fixado no êxito ou nulo na recusa.
 * Razão: cudaMemcpyAsync só prova DMA sem estágio para memória fixada.
 */
void *reservar_memoria_intermediaria_cuda(uint32_t quantidade_de_bytes)
{
    void *memoria = 0;

    if (quantidade_de_bytes == 0 ||
        cudaHostAlloc(&memoria, (size_t)quantidade_de_bytes,
                      cudaHostAllocPortable) != cudaSuccess) {
        return 0;
    }
    return memoria;
}

/*
 * COROLLARIO DA RESTITUICAO INTERMEDIARIA
 * Proposito: devolver á execução CUDA uma região CPU fixada.
 * Pre-condições: endereço nasceu de cudaHostAlloc ou é nulo.
 * Effeitos: chama cudaFreeHost. Retorno: unidade ou zero na recusa.
 * Razão: o nulo não representa posse e, portanto, já está restituído.
 */
int destruir_memoria_intermediaria_cuda(void *memoria)
{
    if (memoria == 0) {
        return 1;
    }
    return cudaFreeHost(memoria) == cudaSuccess;
}

/*
 * LEMMA DA REGIAO CUDA
 * Proposito: julgar limites sem somma susceptível a transbordar.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: unidade ou zero.
 * Razão: primeiro se ordena o deslocamento; depois se subtrahe a capacidade.
 */
static int regiao_cuda_e_valida(const struct transportador_cuda *transportador,
                                uint64_t deslocamento, uint32_t quantidade)
{
    return transportador != 0 && transportador->meio != 0 &&
           transportador->corrente != 0 && quantidade != 0 &&
           deslocamento <= transportador->meio->capacidade_em_bytes &&
           quantidade <=
               transportador->meio->capacidade_em_bytes - deslocamento;
}

/*
 * THEOREMA DA LEITURA POR DMA
 * Proposito: mover região da VRAM para destino CPU fixado.
 * Pre-condições: transportador vivo, destino fixado e região contida.
 * Effeitos: submette cudaMemcpyAsync e synchroniza a corrente.
 * Retorno: unidade somente se submissão e termo alcançam êxito.
 * Razão: a conclusão posterior só reutilizará memória já preenchida.
 */
int ler_meio_cuda(struct transportador_cuda *transportador,
                  uint64_t deslocamento, void *destino,
                  uint32_t quantidade_de_bytes)
{
    if (destino == 0 || !regiao_cuda_e_valida(
            transportador, deslocamento, quantidade_de_bytes)) return 0;
    if (cudaSetDevice(transportador->meio->indice_da_gpu) != cudaSuccess ||
        cudaMemcpyAsync(destino,
                        transportador->meio->memoria_da_gpu +
                            (size_t)deslocamento,
                        (size_t)quantidade_de_bytes, cudaMemcpyDeviceToHost,
                        transportador->corrente) != cudaSuccess) return 0;
    return cudaStreamSynchronize(transportador->corrente) == cudaSuccess;
}

/*
 * THEOREMA DA ESCRIPTA POR DMA
 * Proposito: mover origem CPU fixada para uma região da VRAM.
 * Pre-condições: transportador vivo, origem fixada e região contida.
 * Effeitos: submette cudaMemcpyAsync e synchroniza a corrente.
 * Retorno: unidade somente se submissão e termo alcançam êxito.
 * Razão: a conclusão posterior só poderá expor dados já depositados.
 */
int escrever_meio_cuda(struct transportador_cuda *transportador,
                       uint64_t deslocamento, const void *origem,
                       uint32_t quantidade_de_bytes)
{
    if (origem == 0 || !regiao_cuda_e_valida(
            transportador, deslocamento, quantidade_de_bytes)) return 0;
    if (cudaSetDevice(transportador->meio->indice_da_gpu) != cudaSuccess ||
        cudaMemcpyAsync(transportador->meio->memoria_da_gpu +
                            (size_t)deslocamento,
                        origem, (size_t)quantidade_de_bytes,
                        cudaMemcpyHostToDevice,
                        transportador->corrente) != cudaSuccess) return 0;
    return cudaStreamSynchronize(transportador->corrente) == cudaSuccess;
}

/*
 * COROLLARIO DA REGIAO CUDA NULA
 * Proposito: apagar região de VRAM sem reservar memória intermediária.
 * Pre-condições: transportador vivo e região contida.
 * Effeitos: submette cudaMemsetAsync e synchroniza a corrente.
 * Retorno: unidade somente se submissão e termo alcançam êxito.
 * Razão: descarte e zero explícito convergem no meio volátil.
 */
int zerar_meio_cuda(struct transportador_cuda *transportador,
                    uint64_t deslocamento, uint32_t quantidade_de_bytes)
{
    if (!regiao_cuda_e_valida(
            transportador, deslocamento, quantidade_de_bytes)) return 0;
    if (cudaSetDevice(transportador->meio->indice_da_gpu) != cudaSuccess ||
        cudaMemsetAsync(transportador->meio->memoria_da_gpu +
                            (size_t)deslocamento,
                        0, (size_t)quantidade_de_bytes,
                        transportador->corrente) != cudaSuccess) return 0;
    return cudaStreamSynchronize(transportador->corrente) == cudaSuccess;
}

/*
 * THEOREMA DA PREPARAÇÃO CUDA COMMUM
 * Proposito: adquirir VRAM, transportadores e sentenças antes do alvo.
 * Pre-condições: destino vazio e configuração pertencente ao domínio.
 * Effeitos: publica contexto completo; restitue toda posse na falha.
 * Retorno: zero no êxito, ou erro negativo sem estado parcial.
 * Razão: filas e conclusões devem possuir morada antes da publicação.
 */
int preparar_meio_assincrono_cuda(
    void **contexto, const struct configuracao_do_apparelho *configuracao)
{
    struct meio_assincrono_cuda *figura;
    size_t quantidade_de_filas;

    if (contexto == 0 || *contexto != 0 || configuracao == 0 ||
        configuracao->capacidade_em_bytes == 0 ||
        configuracao->quantidade_de_filas <= 0) return -EINVAL;
    figura = calloc(1, sizeof(*figura));
    if (figura == 0) return -ENOMEM;
    if (!criar_meio_cuda(&figura->meio, configuracao->indice_da_gpu,
                         configuracao->capacidade_em_bytes)) {
        free(figura);
        return -ENOMEM;
    }
    quantidade_de_filas = (size_t)configuracao->quantidade_de_filas;
    figura->transportadores = calloc(
        quantidade_de_filas, sizeof(*figura->transportadores));
    figura->conclusoes = calloc(
        quantidade_de_filas, sizeof(*figura->conclusoes));
    if (figura->transportadores == 0 || figura->conclusoes == 0) {
        free(figura->conclusoes);
        free(figura->transportadores);
        destruir_meio_cuda(&figura->meio);
        free(figura);
        return -ENOMEM;
    }
    figura->quantidade_de_filas = configuracao->quantidade_de_filas;
    *contexto = figura;
    return 0;
}

/*
 * COROLLARIO DA RESTITUIÇÃO CUDA COMMUM
 * Proposito: desfazer em ordem inversa as posses do contexto CUDA.
 * Pre-condições: nenhuma sentença ou transporte permanece pendente.
 * Effeitos: restitue correntes, VRAM, taboas e invólucro.
 * Retorno: nenhum; o termo futuro publicará diagnóstico pormenorizado.
 * Razão: cada corrente morre antes do reservatório que ella referencia.
 */
void destruir_meio_assincrono_cuda(void *contexto)
{
    struct meio_assincrono_cuda *figura = contexto;
    int indice;

    if (figura == 0) return;
    for (indice = 0; indice < figura->quantidade_de_filas; ++indice)
        destruir_transportador_cuda(&figura->transportadores[indice]);
    destruir_meio_cuda(&figura->meio);
    free(figura->conclusoes);
    free(figura->transportadores);
    free(figura);
}

/*
 * LEMMA DO TRANSPORTADOR NUMERADO
 * Proposito: achar a corrente pertencente a uma fila válida.
 * Pre-condições: nenhuma; contexto e índice poderão ser estranhos.
 * Effeitos: nenhum. Retorno: transportador ou nulo fora do domínio.
 * Razão: a fronteira da taboa antecede todo accesso por índice.
 */
static struct transportador_cuda *achar_transportador_cuda(
    struct meio_assincrono_cuda *figura, int indice_da_fila)
{
    if (figura == 0 || indice_da_fila < 0 ||
        indice_da_fila >= figura->quantidade_de_filas) return 0;
    return &figura->transportadores[indice_da_fila];
}

/*
 * THEOREMA DO VÍNCULO CUDA
 * Proposito: fazer nascer a corrente no fio que governará sua fila.
 * Pre-condições: contexto vivo e índice pertencente á configuração.
 * Effeitos: adquire uma corrente singular quando ainda não existe.
 * Retorno: zero no êxito, ou erro negativo sem posse parcial.
 * Razão: o contexto CUDA deverá ser assentado pelo proprio fio da fila.
 */
int vincular_fila_do_meio_cuda(void *contexto, int indice_da_fila)
{
    struct meio_assincrono_cuda *figura = contexto;
    struct transportador_cuda *transportador = achar_transportador_cuda(
        figura, indice_da_fila);

    if (transportador == 0) return -EINVAL;
    if (transportador->corrente != 0) return 0;
    return criar_transportador_cuda(transportador, &figura->meio) ? 0 : -EIO;
}

/*
 * THEOREMA DO PRIMEIRO DMA
 * Proposito: percorrer as duas direcções e a zeragem antes do alvo.
 * Pre-condições: fila válida e memória CPU previamente fixada.
 * Effeitos: transporta zeros pela corrente e torna a apagá-los na VRAM.
 * Retorno: zero no êxito, -EINVAL no domínio ou -EIO no transporte.
 * Razão: cada caminho material deverá fallir antes de receber trabalho.
 */
int aquecer_fila_do_meio_cuda(void *contexto, int indice_da_fila,
                              void *memoria, size_t quantidade_de_bytes)
{
    struct meio_assincrono_cuda *figura = contexto;
    struct transportador_cuda *transportador;
    uint32_t quantidade;

    if (vincular_fila_do_meio_cuda(contexto, indice_da_fila) < 0 ||
        memoria == 0 || quantidade_de_bytes == 0 ||
        quantidade_de_bytes > UINT32_MAX) return -EINVAL;
    transportador = achar_transportador_cuda(figura, indice_da_fila);
    quantidade = (uint32_t)quantidade_de_bytes;
    memset(memoria, 0, quantidade_de_bytes);
    if (!escrever_meio_cuda(transportador, 0, memoria, quantidade) ||
        !ler_meio_cuda(transportador, 0, memoria, quantidade) ||
        !zerar_meio_cuda(transportador, 0, quantidade)) return -EIO;
    return 0;
}

/*
 * LEMMA DA SENTENÇA CUDA NUMERADA
 * Proposito: achar a conclusão pertencente a uma fila válida.
 * Pre-condições: nenhuma; contexto e índice poderão ser estranhos.
 * Effeitos: nenhum. Retorno: sentença ou nulo fora do domínio.
 * Razão: uma só demonstração cerca a taboa paralella ás correntes.
 */
static struct conclusao_cuda *achar_conclusao_cuda(
    struct meio_assincrono_cuda *figura, int indice_da_fila)
{
    if (figura == 0 || indice_da_fila < 0 ||
        indice_da_fila >= figura->quantidade_de_filas) return 0;
    return &figura->conclusoes[indice_da_fila];
}

/*
 * THEOREMA DA LEITURA CUDA PROMETTIDA
 * Proposito: transportar por DMA e differir a sentença até a colheita.
 * Pre-condições: fila ociosa, destino fixado e região contida.
 * Effeitos: preenche o destino e arma exactamente uma conclusão.
 * Retorno: zero quando acceita, ou erro negativo sem promessa ulterior.
 * Razão: a taboa já separa tempos ainda que a execução espere a corrente.
 */
int submeter_leitura_ao_meio_cuda(
    void *contexto, int indice_da_fila, uint64_t deslocamento, void *destino,
    uint32_t quantidade_de_bytes, funcao_de_conclusao_do_meio concluir,
    void *argumento)
{
    struct meio_assincrono_cuda *figura = contexto;
    struct transportador_cuda *transportador = achar_transportador_cuda(
        figura, indice_da_fila);
    struct conclusao_cuda *conclusao = achar_conclusao_cuda(
        figura, indice_da_fila);

    if (conclusao == 0 || concluir == 0 || destino == 0 ||
        !regiao_cuda_e_valida(transportador, deslocamento,
                              quantidade_de_bytes)) return -EINVAL;
    if (conclusao->pendente) return -EBUSY;
    if (!ler_meio_cuda(transportador, deslocamento, destino,
                       quantidade_de_bytes)) return -EIO;
    conclusao->concluir = concluir;
    conclusao->argumento = argumento;
    conclusao->erro = 0;
    conclusao->pendente = 1;
    return 0;
}

/*
 * THEOREMA DA ESCRIPTA CUDA PROMETTIDA
 * Proposito: depositar por DMA e differir a sentença até a colheita.
 * Pre-condições: fila ociosa, origem fixada e região contida.
 * Effeitos: altera a VRAM e arma exactamente uma conclusão.
 * Retorno: zero quando acceita, ou erro negativo sem promessa ulterior.
 * Razão: ambas as direcções submettem-se á mesma ordem temporal.
 */
int submeter_escripta_ao_meio_cuda(
    void *contexto, int indice_da_fila, uint64_t deslocamento,
    const void *origem, uint32_t quantidade_de_bytes,
    funcao_de_conclusao_do_meio concluir, void *argumento)
{
    struct meio_assincrono_cuda *figura = contexto;
    struct transportador_cuda *transportador = achar_transportador_cuda(
        figura, indice_da_fila);
    struct conclusao_cuda *conclusao = achar_conclusao_cuda(
        figura, indice_da_fila);

    if (conclusao == 0 || concluir == 0 || origem == 0 ||
        !regiao_cuda_e_valida(transportador, deslocamento,
                              quantidade_de_bytes)) return -EINVAL;
    if (conclusao->pendente) return -EBUSY;
    if (!escrever_meio_cuda(transportador, deslocamento, origem,
                            quantidade_de_bytes)) return -EIO;
    conclusao->concluir = concluir;
    conclusao->argumento = argumento;
    conclusao->erro = 0;
    conclusao->pendente = 1;
    return 0;
}

/*
 * COROLLARIO DO ZERO CUDA PROMETTIDO
 * Proposito: apagar a VRAM e differir a sentença até a colheita.
 * Pre-condições: fila ociosa e região contida no reservatório.
 * Effeitos: reduz a região a zero e arma exactamente uma conclusão.
 * Retorno: zero quando acceita, ou erro negativo sem promessa ulterior.
 * Razão: descarte e zero explícito partilham uma só operação material.
 */
int submeter_zeragem_ao_meio_cuda(
    void *contexto, int indice_da_fila, uint64_t deslocamento,
    uint32_t quantidade_de_bytes, funcao_de_conclusao_do_meio concluir,
    void *argumento)
{
    struct meio_assincrono_cuda *figura = contexto;
    struct transportador_cuda *transportador = achar_transportador_cuda(
        figura, indice_da_fila);
    struct conclusao_cuda *conclusao = achar_conclusao_cuda(
        figura, indice_da_fila);

    if (conclusao == 0 || concluir == 0 ||
        !regiao_cuda_e_valida(transportador, deslocamento,
                              quantidade_de_bytes)) return -EINVAL;
    if (conclusao->pendente) return -EBUSY;
    if (!zerar_meio_cuda(transportador, deslocamento,
                         quantidade_de_bytes)) return -EIO;
    conclusao->concluir = concluir;
    conclusao->argumento = argumento;
    conclusao->erro = 0;
    conclusao->pendente = 1;
    return 0;
}

/*
 * THEOREMA DA COLHEITA CUDA SINGULAR
 * Proposito: entregar a promessa pendente da fila exactamente uma vez.
 * Pre-condições: orçamento positivo e fila pertencente ao contexto.
 * Effeitos: desarma a sentença antes de chamar o consulente.
 * Retorno: uma conclusão, zero na ausência ou erro negativo no domínio.
 * Razão: a ordem permitte que a própria conclusão arme trabalho novo.
 */
int colher_meio_cuda(void *contexto, int indice_da_fila, int orcamento)
{
    struct meio_assincrono_cuda *figura = contexto;
    struct conclusao_cuda *conclusao = achar_conclusao_cuda(
        figura, indice_da_fila);
    funcao_de_conclusao_do_meio concluir;
    void *argumento;
    int erro;

    if (conclusao == 0 || orcamento <= 0) return -EINVAL;
    if (!conclusao->pendente) return 0;
    concluir = conclusao->concluir;
    argumento = conclusao->argumento;
    erro = conclusao->erro;
    conclusao->concluir = 0;
    conclusao->argumento = 0;
    conclusao->erro = 0;
    conclusao->pendente = 0;
    concluir(argumento, erro);
    return 1;
}

/*
 * COROLLARIO DA TABOA CUDA
 * Proposito: reunir todas as operações sob o contracto commum.
 * Pre-condições: as funcções conservam suas assinaturas normativas.
 * Effeitos: nenhum. Retorno: endereço estático e immutável da taboa.
 * Razão: o alvo conhece a lei e ignora a execução material de CUDA.
 */
const struct operacoes_do_meio *obter_operacoes_do_meio_cuda(void)
{
    static const struct operacoes_do_meio operacoes = {
        .preparar = preparar_meio_assincrono_cuda,
        .vincular_fila = vincular_fila_do_meio_cuda,
        .aquecer_fila = aquecer_fila_do_meio_cuda,
        .destruir = destruir_meio_assincrono_cuda,
        .ler = submeter_leitura_ao_meio_cuda,
        .escrever = submeter_escripta_ao_meio_cuda,
        .zerar = submeter_zeragem_ao_meio_cuda,
        .colher = colher_meio_cuda
    };

    return &operacoes;
}
