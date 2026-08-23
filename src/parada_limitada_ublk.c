#include "parada_limitada_ublk.h"

#include <errno.h>
#include <fcntl.h>
#include <liburing.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define EXTENSAO_DO_CAMINHO_UBLKC 32U
#define IDENTIDADE_DA_ORDEM 1ULL
#define IDENTIDADE_DO_PRAZO 2ULL

/*
 * LEMMA DA ORDEM CERCADA
 * Proposito: gravar STOP_DEV e sua identidade numa SQE de extensão dupla.
 * Pre-condições: SQE128, informação, descritor e caminho julgados.
 * Effeitos: preenche a ordem que o núcleo ublk receberá.
 * Retorno: nenhum. Razão: a codificação depende da faculdade anunciada.
 */
static void preparar_ordem_de_parada(
    struct io_uring_sqe *sqe, int descritor,
    const struct ublksrv_ctrl_dev_info *informacao, char *caminho)
{
    struct ublksrv_ctrl_cmd *ordem;
    uint64_t codigo = informacao->flags & UBLK_F_CMD_IOCTL_ENCODE ?
        UBLK_U_CMD_STOP_DEV : UBLK_CMD_STOP_DEV;

    memset(sqe, 0, sizeof(*sqe) * 2U);
    sqe->fd = descritor;
    sqe->opcode = IORING_OP_URING_CMD;
    sqe->flags = IOSQE_IO_LINK;
    memcpy(&sqe->off, &codigo, sizeof(codigo));
    ordem = (struct ublksrv_ctrl_cmd *)&sqe->addr3;
    ordem->dev_id = informacao->dev_id;
    ordem->queue_id = (__u16)-1;
    if (caminho != 0) {
        ordem->addr = (__u64)(uintptr_t)caminho;
        ordem->len = EXTENSAO_DO_CAMINHO_UBLKC;
        ordem->dev_path_len = EXTENSAO_DO_CAMINHO_UBLKC;
    }
    io_uring_sqe_set_data64(sqe, IDENTIDADE_DA_ORDEM);
}

/*
 * THEOREMA DA PARADA LIMITADA
 * Proposito: ligar STOP_DEV a um relógio do núcleo e colher sua sentença.
 * Pre-condições: controle vivo e duração positiva representável.
 * Effeitos: possue porta e anel próprios; conserva-os se a ordem não cessa.
 * Retorno: resultado do comando, -ETIMEDOUT ou erro negativo da fronteira.
 * Razão: no vencimento, vazar posses é mais seguro que esperar ou libertá-las.
 */
int parar_dispositivo_ublk_com_limite(
    const struct ublksrv_ctrl_dev *controle, uint64_t duracao_em_nanossegundos)
{
    const struct ublksrv_ctrl_dev_info *informacao;
    struct __kernel_timespec prazo;
    struct io_uring_params parametros = {0};
    struct io_uring anel;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *conclusao;
    char *caminho = 0;
    int descritor;
    int resultado;

    if (controle == 0 || duracao_em_nanossegundos == 0) return -EINVAL;
    informacao = ublksrv_ctrl_get_dev_info(controle);
    if (informacao == 0) return -ENODEV;
    if (informacao->flags & UBLK_F_UNPRIVILEGED_DEV) {
        caminho = calloc(EXTENSAO_DO_CAMINHO_UBLKC, 1);
        if (caminho == 0) return -ENOMEM;
        (void)snprintf(caminho, EXTENSAO_DO_CAMINHO_UBLKC,
                       "/dev/ublkc%u", informacao->dev_id);
    }
    descritor = open("/dev/ublk-control", O_RDWR | O_CLOEXEC);
    if (descritor < 0) { free(caminho); return -errno; }
    parametros.flags = IORING_SETUP_SQE128;
    resultado = io_uring_queue_init_params(2, &anel, &parametros);
    if (resultado < 0) { close(descritor); free(caminho); return resultado; }
    sqe = io_uring_get_sqe(&anel);
    if (sqe == 0) { resultado = -ENOBUFS; goto restituir; }
    preparar_ordem_de_parada(sqe, descritor, informacao, caminho);
    sqe = io_uring_get_sqe(&anel);
    if (sqe == 0) { resultado = -ENOBUFS; goto restituir; }
    prazo.tv_sec = (__kernel_time64_t)(duracao_em_nanossegundos / 1000000000ULL);
    prazo.tv_nsec = (long)(duracao_em_nanossegundos % 1000000000ULL);
    io_uring_prep_link_timeout(sqe, &prazo, 0);
    io_uring_sqe_set_data64(sqe, IDENTIDADE_DO_PRAZO);
    resultado = io_uring_submit(&anel);
    if (resultado > 0 && resultado != 2) return -EIO;
    if (resultado != 2) goto restituir;
    for (;;) {
        do resultado = io_uring_wait_cqe(&anel, &conclusao);
        while (resultado == -EINTR);
        if (resultado < 0) goto restituir;
        resultado = conclusao->res;
        if (io_uring_cqe_get_data64(conclusao) == IDENTIDADE_DO_PRAZO &&
            resultado == -ECANCELED) {
            io_uring_cqe_seen(&anel, conclusao);
            continue;
        }
        if (io_uring_cqe_get_data64(conclusao) == IDENTIDADE_DO_PRAZO &&
            resultado == -ETIME) {
            io_uring_cqe_seen(&anel, conclusao);
            return -ETIMEDOUT;
        }
        io_uring_cqe_seen(&anel, conclusao);
        if (resultado == -ECANCELED) resultado = -ETIMEDOUT;
        break;
    }
restituir:
    io_uring_queue_exit(&anel);
    close(descritor);
    free(caminho);
    return resultado;
}
