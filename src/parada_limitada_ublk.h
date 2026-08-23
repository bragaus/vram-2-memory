#ifndef PARADA_LIMITADA_UBLK_H
#define PARADA_LIMITADA_UBLK_H

#include <stdint.h>
#include <ublksrv.h>

/*
 * Proposito: ordenar STOP_DEV sob um prazo relativo conhecido.
 * Pre-condições: controle vivo e duração positiva em nanossegundos.
 * Effeitos: abre uma porta própria e submette ordem ligada ao timeout.
 * Retorno: resultado do núcleo, -ETIMEDOUT ou outro erro negativo.
 * Razão: a porta ordinária da bibliotheca espera sem limite exterior.
 */
int parar_dispositivo_ublk_com_limite(
    const struct ublksrv_ctrl_dev *controle, uint64_t duracao_em_nanossegundos);

#endif
