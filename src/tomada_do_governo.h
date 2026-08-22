#ifndef TOMADA_DO_GOVERNO_H
#define TOMADA_DO_GOVERNO_H

#include <sys/un.h>

/*
 * Proposito: converter caminho textual na figura nativa de uma tomada Unix.
 * Pre-condições: destino e caminho vivos. Effeitos: publica figura completa.
 * Retorno: zero ou erro negativo do domínio. Razão: um só limite sun_path.
 */
int formar_endereco_da_tomada(struct sockaddr_un *destino,
                              const char *caminho);

/*
 * Proposito: publicar uma tomada servidora no caminho exclusivo da instância.
 * Pre-condições: recinto possuído e caminho ainda inexistente.
 * Effeitos: liga tomada 0660 e escuta; jamais remove homônimo anterior.
 * Retorno: descritor no êxito ou erro negativo do domínio ou do systema.
 * Razão: bind sem unlink cego preserva a prova exterior de outra posse.
 */
int abrir_tomada_servidora_do_governo(const char *caminho);

/*
 * Proposito: ligar um cliente á tomada servidora de uma instância.
 * Pre-condições: caminho canônico e servidor escutando.
 * Effeitos: abre ligação bloqueante com close-on-exec.
 * Retorno: descritor no êxito ou erro negativo do domínio ou do systema.
 * Razão: o cliente governa sem adquirir o recinto ou o meio de dados.
 */
int ligar_tomada_do_governo(const char *caminho);

/*
 * Proposito: restituir a tomada publicada pelo proprietário chamador.
 * Pre-condições: caminho pertence ao recinto possuído. Effeitos: remove-o.
 * Retorno: zero quando removido ou ausente, senão erro negativo do systema.
 * Razão: o limite textual torna explícito o alcance da limpeza.
 */
int restituir_tomada_do_governo(const char *caminho);

#endif
