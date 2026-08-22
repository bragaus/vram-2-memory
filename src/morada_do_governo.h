#ifndef MORADA_DO_GOVERNO_H
#define MORADA_DO_GOVERNO_H

#define RAIZ_ORDINARIA_DO_GOVERNO "/run/vramdisk"
#define LIMITE_DO_CAMINHO_DO_GOVERNO 4096U

/* A instância conserva três caminhos derivados da mesma raiz e número. */
struct morada_do_governo {
    char directorio[LIMITE_DO_CAMINHO_DO_GOVERNO];
    char tomada[LIMITE_DO_CAMINHO_DO_GOVERNO];
    char processo[LIMITE_DO_CAMINHO_DO_GOVERNO];
};

/*
 * Proposito: formar directório, tomada e registro de uma instância numerada.
 * Pre-condições: raiz absoluta, destino vivo e índice não negativo.
 * Effeitos: publica os três caminhos somente quando todos cabem.
 * Retorno: zero no êxito ou erro negativo sem figura parcial.
 * Razão: uma derivação singular impede cliente e servidor de divergir.
 */
int formar_morada_do_governo(struct morada_do_governo *destino,
                             const char *raiz, unsigned int indice);

/*
 * Proposito: crear a raiz compartilhada e o directório exclusivo da instância.
 * Pre-condições: morada formada e raiz absoluta sem barra derradeira.
 * Effeitos: crea directórios com modos 0750, sem adoptar instância existente.
 * Retorno: zero no êxito ou erro negativo do systema.
 * Razão: exclusividade do directório antecede tomada e registro de processo.
 */
int preparar_morada_do_governo(const struct morada_do_governo *morada,
                               const char *raiz);

#endif
