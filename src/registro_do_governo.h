#ifndef REGISTRO_DO_GOVERNO_H
#define REGISTRO_DO_GOVERNO_H

#include <sys/types.h>

/*
 * Proposito: publicar o número do processo proprietário numa folha exclusiva.
 * Pre-condições: caminho em recinto possuído e processo estritamente positivo.
 * Effeitos: crea registro 0640 e grava uma linha decimal integral.
 * Retorno: zero no êxito ou erro negativo, sem conservar folha incompleta.
 * Razão: o registro diagnostica a posse sem substituir a tomada como verdade.
 */
int publicar_registro_do_governo(const char *caminho, pid_t processo);

/*
 * Proposito: restituir um registro anteriormente publicado pelo chamador.
 * Pre-condições: caminho pertence ao recinto que o chamador possue.
 * Effeitos: remove a folha; ausência já representa restituição completa.
 * Retorno: zero no êxito ou erro negativo do systema.
 * Razão: uma operação estreita torna explícito o limite da limpeza.
 */
int restituir_registro_do_governo(const char *caminho);

#endif
