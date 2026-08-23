#include "../src/configuracao.h"
#include "../src/reserva_de_buffers.h"

#include <errno.h>
#include <stdint.h>

/*
 * Proposito: provar alinhamento, quinhões singulares, zero e restituição.
 * Pre-condições: o bloco fundamental mede 4096 octetos.
 * Effeitos: adquire e restitue uma região de seis buffers.
 * Retorno: zero na convergência ou unidade na primeira contradicção.
 * Razão: vizinhança e índices hostis revelam sobreposição ou escolha tardia.
 */
int main(void)
{
    struct reserva_de_buffers reserva = {0};
    unsigned char *primeiro;
    unsigned char *derradeiro;

    if (criar_reserva_de_buffers(&reserva, 2, 3, 4096) != 0 ||
        reserva.quantidade_em_bytes != 24576 ||
        (uintptr_t)reserva.inicio % TAMANHO_DO_BLOCO_EM_BYTES != 0 ||
        criar_reserva_de_buffers(&reserva, 2, 3, 4096) != -EINVAL)
        return 1;
    primeiro = achar_buffer_reservado(&reserva, 0, 0, 4096);
    derradeiro = achar_buffer_reservado(&reserva, 1, 2, 4096);
    if (primeiro != reserva.inicio ||
        derradeiro != reserva.inicio + 5 * 4096 ||
        achar_buffer_reservado(&reserva, 2, 0, 1) != 0 ||
        achar_buffer_reservado(&reserva, 0, 3, 1) != 0 ||
        achar_buffer_reservado(&reserva, 0, 0, 4097) != 0)
        return 1;
    for (uint64_t indice = 0; indice < reserva.quantidade_em_bytes; indice++)
        if (reserva.inicio[indice] != 0) return 1;
    primeiro[4095] = 29;
    if (primeiro[4096] != 0) return 1;
    destruir_reserva_de_buffers(&reserva);
    destruir_reserva_de_buffers(&reserva);
    return reserva.inicio != 0 || reserva.quantidade_em_bytes != 0;
}
