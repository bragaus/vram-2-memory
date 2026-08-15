COMPILADOR ?= cc
AVISOS := -std=c11 -Wall -Wextra -Wpedantic -Werror
DIRECTORIO_DA_CONSTRUCAO := construcao
PROVAS := $(DIRECTORIO_DA_CONSTRUCAO)/provar_transicoes \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_configuracao \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_meio_simulado \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_fila_de_requisicoes

.PHONY: provar preparar_ublk limpar

provar: $(PROVAS)
	@for prova in $(PROVAS); do $$prova; done

$(DIRECTORIO_DA_CONSTRUCAO):
	mkdir -p $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_transicoes: \
		testes/provar_transicoes.c src/estado_da_requisicao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_configuracao: \
		testes/provar_configuracao.c src/configuracao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_meio_simulado: \
		testes/provar_meio_simulado.c src/meio_simulado.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_fila_de_requisicoes: \
		testes/provar_fila_de_requisicoes.c src/fila_de_requisicoes.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

preparar_ublk: $(DIRECTORIO_DA_CONSTRUCAO)/alvo_ublk.o

$(DIRECTORIO_DA_CONSTRUCAO)/alvo_ublk.o: src/alvo_ublk.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $$(pkg-config --cflags ublksrv) -c $< -o $@

limpar:
	rm -f $(PROVAS)
