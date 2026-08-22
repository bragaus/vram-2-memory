#define _POSIX_C_SOURCE 200809L
#include "tomada_do_governo.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

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

/*
 * PROPOSICAO DA TOMADA EXCLUSIVA
 * Proposito: ligar e pôr em escuta a tomada de um recinto possuído.
 * Pre-condições: caminho absoluto ainda inexistente. Effeitos: publica 0660.
 * Retorno: descritor no êxito ou erro negativo com recurso restituído.
 * Razão: somente um bind victorioso concede licença para remover a tomada.
 */
int abrir_tomada_servidora_do_governo(const char *caminho)
{
    struct sockaddr_un endereco;
    int descritor;
    int resultado;
    int erro;

    resultado = formar_endereco_da_tomada(&endereco, caminho);
    if (resultado < 0) return resultado;
    descritor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (descritor < 0) return -errno;
    if (bind(descritor, (const struct sockaddr *)&endereco,
             sizeof(endereco)) != 0) {
        erro = errno;
        (void)close(descritor);
        return -erro;
    }
    if (chmod(caminho, 0660) == 0 && listen(descritor, 8) == 0)
        return descritor;
    erro = errno;
    (void)close(descritor);
    (void)unlink(caminho);
    return -erro;
}
