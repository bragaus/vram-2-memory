#define _POSIX_C_SOURCE 200809L
#include "instancia_do_servidor.h"
#include "registro_do_governo.h"
#include "tomada_do_governo.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
/*
 * COROLLARIO DA RESTITUICAO MARCADA
 * Proposito: restituir em ordem inversa todas as posses declaradas.
 * Pre-condições: figura formada por abrir_instancia_do_servidor.
 * Effeitos: tenta cada limpeza mesmo depois de falha anterior.
 * Retorno: zero ou primeiro erro negativo.
 * Razão: marcas prohibem tanto omissão quanto remoção de posse alheia.
 */
int fechar_instancia_do_servidor(struct instancia_do_servidor *instancia)
{
    int primeiro = 0;
    int resultado;

    if (instancia == 0) return -EINVAL;
    if (instancia->governo_preparado) {
        if (instancia->governo.fio_nascido) {
            resultado = destruir_apparelho_governado(&instancia->governo);
            if (primeiro == 0 && resultado < 0) primeiro = resultado;
        }
        resultado = encerrar_governo_do_apparelho(&instancia->governo);
        if (primeiro == 0 && resultado < 0) primeiro = resultado;
        if (resultado == 0) instancia->governo_preparado = 0;
    }
    if (instancia->tomada_servidora >= 0) {
        if (close(instancia->tomada_servidora) != 0 && primeiro == 0)
            primeiro = -errno;
        instancia->tomada_servidora = -1;
    }
    if (instancia->recinto_preparado) {
        resultado = restituir_tomada_do_governo(instancia->morada.tomada);
        if (primeiro == 0 && resultado < 0) primeiro = resultado;
    }
    if (instancia->registro_publicado) {
        resultado = restituir_registro_do_governo(instancia->morada.processo);
        if (primeiro == 0 && resultado < 0) primeiro = resultado;
        instancia->registro_publicado = 0;
    }
    if (instancia->recinto_preparado &&
        rmdir(instancia->morada.directorio) != 0 && errno != ENOENT &&
        primeiro == 0) primeiro = -errno;
    instancia->recinto_preparado = 0;
    return primeiro;
}

/*
 * THEOREMA DA INSTANCIA INTEIRA
 * Proposito: adquirir em ordem recinto, registro, tomada e governo.
 * Pre-condições: raiz, índice e faculdades válidos.
 * Effeitos: publica figura completa ou restitue toda posse parcial.
 * Retorno: zero ou primeiro erro negativo da acquisição.
 * Razão: a audiência só principia depois dos quatro sellos victoriosos.
 */
int abrir_instancia_do_servidor(
    struct instancia_do_servidor *instancia, const char *raiz,
    unsigned int indice, servico_do_apparelho servir,
    termo_do_apparelho terminar, void *contexto)
{
    int resultado;

    if (instancia == 0) return -EINVAL;
    memset(instancia, 0, sizeof(*instancia));
    instancia->tomada_servidora = -1;
    resultado = formar_morada_do_governo(&instancia->morada, raiz, indice);
    if (resultado < 0) return resultado;
    resultado = preparar_morada_do_governo(&instancia->morada, raiz);
    if (resultado < 0) return resultado;
    instancia->recinto_preparado = 1;
    resultado = publicar_registro_do_governo(
        instancia->morada.processo, getpid());
    if (resultado < 0) goto falha;
    instancia->registro_publicado = 1;
    instancia->tomada_servidora = abrir_tomada_servidora_do_governo(
        instancia->morada.tomada);
    if (instancia->tomada_servidora < 0) {
        resultado = instancia->tomada_servidora;
        goto falha;
    }
    resultado = preparar_governo_do_apparelho(
        &instancia->governo, servir, terminar, contexto);
    if (resultado < 0) goto falha;
    instancia->governo_preparado = 1;
    return 0;
falha:
    (void)fechar_instancia_do_servidor(instancia);
    return resultado;
}
