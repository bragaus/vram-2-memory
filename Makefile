COMPILADOR ?= cc
DIRECTORIO_DO_CUDA ?= /usr/local/cuda
AVISOS := -std=c11 -Wall -Wextra -Wpedantic -Werror
DIRECTORIO_DA_CONSTRUCAO := construcao
PROVAS := $(DIRECTORIO_DA_CONSTRUCAO)/provar_transicoes \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_configuracao \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_meio_simulado \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_fila_de_requisicoes
FONTES_DO_SERVIDOR := src/principal.c src/servidor_ublk.c src/alvo_ublk.c \
	src/configuracao.c src/estado_da_requisicao.c src/meio_simulado.c \
	src/meio_cuda.c src/fila_de_requisicoes.c
SERVIDOR := $(DIRECTORIO_DA_CONSTRUCAO)/vram-2-memory

.PHONY: provar preparar_ublk preparar_cuda limpar

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

preparar_ublk: $(SERVIDOR)
preparar_cuda: $(SERVIDOR)

$(SERVIDOR): $(FONTES_DO_SERVIDOR) | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) -I$(DIRECTORIO_DO_CUDA)/include \
		$$(pkg-config --cflags ublksrv) $^ -o $@ \
		$$(pkg-config --libs ublksrv) -L$(DIRECTORIO_DO_CUDA)/lib64 \
		-Wl,-rpath,$(DIRECTORIO_DO_CUDA)/lib64 -lcudart -pthread

limpar:
	rm -f $(PROVAS) $(SERVIDOR)
