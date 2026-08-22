#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

/*
 * Proposito: pedir descarte seguro, operação ausente do alvo ublk autoral.
 * Pre-condições: argumento nomeia exactamente o bloco creado nesta VM.
 * Effeitos: abre o bloco e envia BLKSECDISCARD sobre o primeiro bloco lógico.
 * Retorno: zero somente quando o núcleo publica EOPNOTSUPP.
 * Razão: uma operação exterior inválida deve chegar a recusa explícita.
 */
int main(int quantidade_de_argumentos, char *argumentos[])
{
    uint64_t intervalo[2] = {0, UINT64_C(4096)};
    int descritor;
    int erro;

    if (quantidade_de_argumentos != 2) {
        fprintf(stderr, "Uso: %s /dev/ublkbN\n", argumentos[0]);
        return 2;
    }
    descritor = open(argumentos[1], O_RDWR | O_CLOEXEC);
    if (descritor < 0) return 3;
    if (ioctl(descritor, BLKSECDISCARD, intervalo) == 0) {
        (void)close(descritor);
        return 4;
    }
    erro = errno;
    if (close(descritor) != 0) return 5;
    if (erro != EOPNOTSUPP) {
        fprintf(stderr, "BLKSECDISCARD recusado com errno %d.\n", erro);
        return 6;
    }
    return 0;
}
