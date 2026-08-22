COMPILADOR ?= cc
DIRECTORIO_DO_CUDA ?= /usr/local/cuda
DISPOSITIVO ?=
AVISOS := -std=c11 -Wall -Wextra -Wpedantic -Werror
DIRECTORIO_DA_CONSTRUCAO := construcao
PROVAS := $(DIRECTORIO_DA_CONSTRUCAO)/provar_transicoes \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_configuracao \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_carga_de_creacao \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_governo_do_apparelho \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_ordens_da_instancia \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_meio_simulado \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_fila_de_requisicoes \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_protocolo_de_governo \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_canal_de_governo \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_morada_do_governo \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_registro_do_governo \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_tomada_do_governo \
	$(DIRECTORIO_DA_CONSTRUCAO)/provar_observatorio
FONTES_DO_SERVIDOR := src/principal.c src/servidor_ublk.c src/alvo_ublk.c \
	src/configuracao.c src/numero_decimal.c src/estado_da_requisicao.c src/meio_simulado.c \
	src/meio_cuda.c src/fila_de_requisicoes.c src/retrato_do_observatorio.c \
	src/monitor_do_observatorio.c src/observador_de_si.c
SERVIDOR := $(DIRECTORIO_DA_CONSTRUCAO)/vram-2-memory
PROVA_CUDA := $(DIRECTORIO_DA_CONSTRUCAO)/provar_meio_cuda
DEMONSTRACAO := $(DIRECTORIO_DA_CONSTRUCAO)/demonstrar_observatorio

.PHONY: provar demonstrar_simulacao provar_cuda provar_pressao preparar_ublk preparar_cuda limpar

provar: $(PROVAS)
	@for prova in $(PROVAS); do $$prova; done

demonstrar_simulacao: $(DEMONSTRACAO)
	$(DEMONSTRACAO)

$(DIRECTORIO_DA_CONSTRUCAO):
	mkdir -p $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_transicoes: \
		testes/provar_transicoes.c src/estado_da_requisicao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_configuracao: \
		testes/provar_configuracao.c src/configuracao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_carga_de_creacao: \
		testes/provar_carga_de_creacao.c src/carga_de_creacao.c src/configuracao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_governo_do_apparelho: \
		testes/provar_governo_do_apparelho.c src/governo_do_apparelho.c src/configuracao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) -pthread $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_ordens_da_instancia: testes/provar_ordens_da_instancia.c \
		src/ordens_da_instancia.c src/governo_do_apparelho.c src/carga_de_creacao.c \
		src/configuracao.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) -pthread $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_meio_simulado: \
		testes/provar_meio_simulado.c src/meio_simulado.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_fila_de_requisicoes: \
		testes/provar_fila_de_requisicoes.c src/fila_de_requisicoes.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_protocolo_de_governo: \
		testes/provar_protocolo_de_governo.c src/protocolo_de_governo.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_canal_de_governo: testes/provar_canal_de_governo.c \
		src/canal_de_governo.c src/protocolo_de_governo.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_morada_do_governo: \
		testes/provar_morada_do_governo.c src/morada_do_governo.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_registro_do_governo: \
		testes/provar_registro_do_governo.c src/registro_do_governo.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_tomada_do_governo: \
		testes/provar_tomada_do_governo.c src/tomada_do_governo.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DIRECTORIO_DA_CONSTRUCAO)/provar_observatorio: testes/provar_observatorio.c \
		src/retrato_do_observatorio.c src/monitor_do_observatorio.c \
		src/observador_de_si.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

$(DEMONSTRACAO): demonstracoes/demonstrar_observatorio.c src/meio_simulado.c \
		src/retrato_do_observatorio.c src/monitor_do_observatorio.c \
		src/observador_de_si.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) $^ -o $@

preparar_ublk: $(SERVIDOR)
preparar_cuda: $(SERVIDOR)
provar_cuda: $(PROVA_CUDA)
	$(PROVA_CUDA)

provar_pressao:
	./testes/provar_pressao_e_swap.sh "$(DISPOSITIVO)"

$(PROVA_CUDA): testes/provar_meio_cuda.c src/meio_cuda.c | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) -I$(DIRECTORIO_DO_CUDA)/include $^ -o $@ \
		-L$(DIRECTORIO_DO_CUDA)/lib64 -Wl,-rpath,$(DIRECTORIO_DO_CUDA)/lib64 -lcudart

$(SERVIDOR): $(FONTES_DO_SERVIDOR) | $(DIRECTORIO_DA_CONSTRUCAO)
	$(COMPILADOR) $(AVISOS) -D_GNU_SOURCE -I$(DIRECTORIO_DO_CUDA)/include \
		-isystem$$(pkg-config --variable=includedir ublksrv) $^ -o $@ \
		$$(pkg-config --libs ublksrv) -L$(DIRECTORIO_DO_CUDA)/lib64 \
		-Wl,-rpath,$(DIRECTORIO_DO_CUDA)/lib64 -lcudart -pthread

limpar:
	rm -f $(PROVAS) $(PROVA_CUDA) $(SERVIDOR) $(DEMONSTRACAO)
