#define _POSIX_C_SOURCE 200809L
#include "registro_do_governo.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/* PROPOSICAO DA FOLHA INTEIRA
 * Proposito: publicar exclusivamente o número decimal do processo.
 * Pre-condições: caminho vivo e processo positivo. Effeitos: crea folha 0640;
 * qualquer falha remove-a. Retorno: zero ou erro negativo do systema.
 * Razão: posse parcial não deve sobreviver como testemunho enganoso.
 */
int publicar_registro_do_governo(const char *caminho, pid_t processo)
{
    char texto[32];
    size_t cursor = 0;
    int descritor;
    int quantidade;
    int erro;
    if (caminho == 0 || processo <= 0) return -EINVAL;
    descritor = open(caminho, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0640);
    if (descritor < 0) return -errno;
    if (fchmod(descritor, 0640) != 0) goto falha;
    quantidade = snprintf(texto, sizeof(texto), "%ld\n", (long)processo);
    if (quantidade <= 0 || (size_t)quantidade >= sizeof(texto)) {
        erro = EOVERFLOW;
        goto restituicao;
    }
    while (cursor < (size_t)quantidade) {
        ssize_t escriptos = write(descritor, texto + cursor,
                                  (size_t)quantidade - cursor);
        if (escriptos > 0) cursor += (size_t)escriptos;
        else if (escriptos < 0 && errno == EINTR) continue;
        else goto falha;
    }
    if (close(descritor) == 0) return 0;
    erro = errno;
    (void)unlink(caminho);
    return -erro;
falha:
    erro = errno;
restituicao:
    (void)close(descritor);
    (void)unlink(caminho);
    return -erro;
}

/*
 * Proposito: restituir a folha que o chamador sabe haver publicado.
 * Pre-condições: caminho pertence ao recinto possuído. Effeitos: remove-o.
 * Retorno: zero quando removido ou ausente, senão erro negativo do systema.
 * Razão: ausência já satisfaz a pós-condição de uma restituição idempotente.
 */
int restituir_registro_do_governo(const char *caminho)
{
    if (caminho == 0) return -EINVAL;
    if (unlink(caminho) == 0 || errno == ENOENT) return 0;
    return -errno;
}
