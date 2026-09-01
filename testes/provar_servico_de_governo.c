#include "../src/servico_de_governo.h"
#include "../src/instancia_do_servidor.h"
#include "../src/canal_de_governo.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/*
 * Proposito: representar o serviço que esta prova não faz nascer.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: a audiência do governo independe de CUDA e ublk.
 */
int servir_prova_do_servico(
    const struct configuracao_do_apparelho *configuracao, void *contexto)
{
    (void)configuracao;
    (void)contexto;
    return 0;
}

/*
 * Proposito: representar o termo que esta prova não precisa exercer.
 * Pre-condições: nenhuma. Effeitos: nenhum. Retorno: sempre zero.
 * Razão: o governo exige ambas as faculdades antes da tomada pública.
 */
int terminar_prova_do_servico(void *contexto)
{
    (void)contexto;
    return 0;
}

struct recado_do_cliente {
    const char *caminho;
    int viu_ok;
};

/*
 * Proposito: abrir uma ligação local á tomada nomeada do governo.
 * Pre-condições: caminho não vazio que caiba em sun_path.
 * Effeitos: crea um descritor ligado. Retorno: descritor ou -1.
 * Razão: cada audiência da prova é uma conexão distinta e efêmera.
 */
static int ligar_ao_governo(const char *caminho)
{
    struct sockaddr_un endereco;
    size_t comprimento;
    int fd;

    comprimento = strlen(caminho);
    if (comprimento == 0 || comprimento >= sizeof(endereco.sun_path)) return -1;
    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;
    memset(&endereco, 0, sizeof(endereco));
    endereco.sun_family = AF_UNIX;
    memcpy(endereco.sun_path, caminho, comprimento + 1);
    if (connect(fd, (struct sockaddr *)&endereco, sizeof(endereco)) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

/*
 * Proposito: conduzir três clientes em série: magia inválida, conecta-e-fecha
 *   e por fim um CONTEMPLAR válido cuja resposta se confere.
 * Pre-condições: recado com caminho vivo. Effeitos: marca viu_ok na terceira.
 * Retorno: nulo. Razão: só o terceiro atendido prova que os dois primeiros não
 *   encerraram o laço do governo.
 */
static void *conduzir_clientes(void *bruto)
{
    struct recado_do_cliente *recado = bruto;
    unsigned char invalido[TAMANHO_DO_CABECALHO_DE_GOVERNO] = {0};
    struct mensagem_de_governo resposta = {0};
    int fd;

    fd = ligar_ao_governo(recado->caminho);
    if (fd >= 0) {
        (void)send(fd, invalido, sizeof(invalido), MSG_NOSIGNAL);
        close(fd);
    }
    fd = ligar_ao_governo(recado->caminho);
    if (fd >= 0) close(fd);
    fd = ligar_ao_governo(recado->caminho);
    if (fd >= 0) {
        if (enviar_mensagem_de_governo(
                fd, OPERACAO_DE_GOVERNO_CONTEMPLAR, 0, 0) == 0 &&
            receber_mensagem_de_governo(fd, &resposta) == 0) {
            uint32_t n = resposta.cabecalho.quantidade_da_carga;
            char texto[256];
            if (resposta.carga != 0 && n < sizeof(texto)) {
                memcpy(texto, resposta.carga, n);
                texto[n] = 0;
                if (strstr(texto, "\"ok\":true") != 0) recado->viu_ok = 1;
            }
        }
        destruir_mensagem_de_governo(&resposta);
        close(fd);
    }
    return 0;
}

/*
 * Proposito: provar que magia inválida e conexão abortada não encerram o laço.
 * Pre-condições: /tmp acolhe recinto efêmero e pthread está disponível.
 * Effeitos: abre uma instância, concede três audiências e restitue tudo.
 * Retorno: unidade quando o laço sobrevive e atende o terceiro cliente.
 * Razão: o laço só deve encerrar por falha interna, jámais por falta do cliente.
 */
int provar_sobrevivencia_do_servico(void)
{
    char molde[] = "/tmp/provar-servico-XXXXXX";
    char raiz[108];
    struct instancia_do_servidor instancia;
    struct recado_do_cliente recado = {0};
    pthread_t fio;
    int resultado = 0;
    int ok = 0;

    if (mkdtemp(molde) == 0) return 0;
    if (snprintf(raiz, sizeof(raiz), "%s/raiz", molde) <= 0) goto limpar_molde;
    if (abrir_instancia_do_servidor(&instancia, raiz, 7,
            servir_prova_do_servico, terminar_prova_do_servico, 0) != 0)
        goto limpar_raiz;
    recado.caminho = instancia.morada.tomada;
    if (pthread_create(&fio, 0, conduzir_clientes, &recado) != 0) goto fechar;
    resultado = conceder_audiencias_do_governo(
        instancia.tomada_servidora, &instancia.governo, 3);
    pthread_join(fio, 0);
    ok = (resultado == 0 && recado.viu_ok == 1);
fechar:
    (void)fechar_instancia_do_servidor(&instancia);
limpar_raiz:
    (void)rmdir(raiz);
limpar_molde:
    (void)rmdir(molde);
    return ok;
}

/*
 * Proposito: reunir as demonstrações do serviço de governo sob um alarme.
 * Pre-condições: nenhuma tomada nomeada exterior é necessária.
 * Effeitos: o alarme converte qualquer travamento em falha visível.
 * Retorno: zero na sobrevivência, unidade na primeira lei contradita.
 * Razão: uma regressão que reencerre o laço travaria a prova sem o alarme.
 */
int main(void)
{
    alarm(15);
    return provar_sobrevivencia_do_servico() ? 0 : 1;
}
