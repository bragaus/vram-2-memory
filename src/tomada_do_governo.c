#define _POSIX_C_SOURCE 200809L
#include "tomada_do_governo.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>

/*
 * THEOREMA DO ENDERECO FINITO
 * Proposito: formar endereço Unix sem truncar seu caminho.
 * Pre-condições: destino e caminho vivos. Effeitos: publica figura julgada.
 * Retorno: zero, -EINVAL ou -ENAMETOOLONG.
 * Razão: truncamento silencioso ligaria o governo a outra morada.
 */
int formar_endereco_da_tomada(struct sockaddr_un *destino,
                              const char *caminho)
{
    struct sockaddr_un figura;
    size_t comprimento;

    if (destino == 0 || caminho == 0 || caminho[0] != '/') return -EINVAL;
    comprimento = strlen(caminho);
    if (comprimento == 0 || comprimento >= sizeof(figura.sun_path))
        return -ENAMETOOLONG;
    memset(&figura, 0, sizeof(figura));
    figura.sun_family = AF_UNIX;
    memcpy(figura.sun_path, caminho, comprimento + 1);
    *destino = figura;
    return 0;
}
