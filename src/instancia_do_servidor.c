#define _POSIX_C_SOURCE 200809L
#include "instancia_do_servidor.h"
#include "registro_do_governo.h"
#include "tomada_do_governo.h"

#include <errno.h>
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
