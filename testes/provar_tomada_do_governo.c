#define _POSIX_C_SOURCE 200809L
#include "../src/tomada_do_governo.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Proposito: provar posse, modo, ligação e restituição numa tomada efêmera.
 * Pre-condições: /tmp acolhe um recinto. Effeitos: crea e restitue a prova.
 * Retorno: nenhum. Razão: o ensaio exterior não toca a morada do serviço.
 */
static void provar_ligacao_exclusiva(void)
{
    char molde[] = "/tmp/provar-tomada-XXXXXX";
    char caminho[108];
    char letra = 0;
    struct stat estado;
    int servidor;
    int cliente;
    int recebido;

    assert(mkdtemp(molde) != 0);
    assert(snprintf(caminho, sizeof(caminho), "%s/control.sock", molde) > 0);
    servidor = abrir_tomada_servidora_do_governo(caminho);
    assert(servidor >= 0);
    assert(stat(caminho, &estado) == 0);
    assert((estado.st_mode & 0777) == 0660);
    assert(abrir_tomada_servidora_do_governo(caminho) == -EADDRINUSE);
    cliente = ligar_tomada_do_governo(caminho);
    assert(cliente >= 0);
    recebido = accept(servidor, 0, 0);
    assert(recebido >= 0);
    assert(send(cliente, "G", 1, 0) == 1);
    assert(recv(recebido, &letra, 1, 0) == 1 && letra == 'G');
    assert(close(recebido) == 0 && close(cliente) == 0);
    assert(close(servidor) == 0);
    assert(restituir_tomada_do_governo(caminho) == 0);
    assert(restituir_tomada_do_governo(caminho) == 0);
    assert(rmdir(molde) == 0);
}

/*
 * Proposito: provar endereço canônico e recusa de caminho excessivo.
 * Pre-condições: nenhuma. Effeitos: termina ao primeiro desvio.
 * Retorno: zero no êxito. Razão: a prova pura antecede qualquer socket.
 */
int main(void)
{
    struct sockaddr_un endereco;
    char excessivo[sizeof(endereco.sun_path) + 2];

    memset(excessivo, 'a', sizeof(excessivo));
    excessivo[0] = '/';
    excessivo[sizeof(excessivo) - 1] = 0;
    assert(formar_endereco_da_tomada(&endereco, "/tmp/governo.sock") == 0);
    assert(endereco.sun_family == AF_UNIX);
    assert(strcmp(endereco.sun_path, "/tmp/governo.sock") == 0);
    assert(formar_endereco_da_tomada(&endereco, "relativo") == -EINVAL);
    assert(formar_endereco_da_tomada(&endereco, excessivo) == -ENAMETOOLONG);
    provar_ligacao_exclusiva();
    return 0;
}
