#ifndef PROTOCOLO_DE_GOVERNO_H
#define PROTOCOLO_DE_GOVERNO_H

#include <stddef.h>
#include <stdint.h>

/* A marca distingue este manuscripto de todo rumor chegado á tomada. */
#define MAGIA_DO_PROTOCOLO_DE_GOVERNO UINT32_C(0x5652414d)
#define VERSAO_DO_PROTOCOLO_DE_GOVERNO UINT16_C(1)
#define TAMANHO_DO_CABECALHO_DE_GOVERNO 12U
#define LIMITE_DA_CARGA_DE_GOVERNO 65536U

/*
 * As operações recebem números permanentes, pois sua ordem futura jámais
 * poderá alterar o sentido de uma mensagem antiga conservada em diagnóstico.
 */
enum operacao_de_governo {
    OPERACAO_DE_GOVERNO_CREAR = 1,
    OPERACAO_DE_GOVERNO_CONTEMPLAR = 2,
    OPERACAO_DE_GOVERNO_DESTRUIR = 3,
    OPERACAO_DE_GOVERNO_ACTIVAR_SWAP = 4,
    OPERACAO_DE_GOVERNO_DESACTIVAR_SWAP = 5
};

/*
 * Retrato nativo de um cabeçalho já julgado; elle não representa sua forma
 * exterior e, portanto, não depende de alinhamento ou ordem da machina.
 */
struct cabecalho_de_governo {
    uint32_t magia;
    uint16_t versao;
    uint16_t operacao;
    uint32_t quantidade_da_carga;
};

/*
 * Proposito: julgar se um número nomeia operação publicada.
 * Pre-condições: nenhuma; todo uint16_t pertence ao domínio de entrada.
 * Effeitos: nenhum. Retorno: unidade quando conhecido, zero no restante.
 * Razão: o servidor jámais executa intenção que seu vocabulário não contém.
 */
int operacao_de_governo_e_conhecida(uint16_t operacao);

#endif
