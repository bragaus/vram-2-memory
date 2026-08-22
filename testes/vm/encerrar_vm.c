#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>

/*
 * Proposito: entregar um código ao dispositivo isa-debug-exit da VM.
 * Pre-condições: argumento entre zero e cento e vinte e sete; porta 0xf4.
 * Effeitos: escreve no porto de saída e, no QEMU, encerra a machina.
 * Retorno: EXIT_FAILURE somente quando a porta não encerra ou é recusada.
 * Razão: o hospedeiro distingue prova verde de panic sem ler prosa do console.
 */
int main(int quantidade_de_argumentos, char *argumentos[])
{
    unsigned long codigo;
    char *fim;

    if (quantidade_de_argumentos != 2) return EXIT_FAILURE;
    errno = 0;
    codigo = strtoul(argumentos[1], &fim, 10);
    if (errno != 0 || *fim != 0 || codigo > 127) return EXIT_FAILURE;
    if (ioperm(0xf4, 4, 1) != 0) {
        perror("ioperm");
        return EXIT_FAILURE;
    }
    outl((unsigned int)codigo, 0xf4);
    return EXIT_FAILURE;
}
