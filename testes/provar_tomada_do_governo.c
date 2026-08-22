#include "../src/tomada_do_governo.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

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
    return 0;
}
