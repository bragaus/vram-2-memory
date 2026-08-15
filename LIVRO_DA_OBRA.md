# LIVRO DA OBRA

> Caderno de marcha do apparelho vram-2-memory, lavrado para que nenhuma proposição seja havida por verdadeira antes que a experiência a demonstre.

## § I. DOS SIGNAES

- `[✓]` — demonstrado por prova executável;
- `[~]` — actualmente sobre a mesa de cálculo;
- `[ ]` — ainda por demonstrar;
- `[!]` — impedido por facto exterior conhecido.

## § II. DO ESTADO PRESENTE

- `[✓]` Proêmio público: natureza volátil e caminho por DMA declarados.
- `[✓]` Contracto dos estados: quatro posições e grandezas da requisição.
- `[~]` Lei das transições: discernir cada passagem legítima entre estados.
- `[ ]` Provas unitárias da lei das transições.
- `[ ]` Configuração do apparelho e limites sem estouro.
- `[ ]` Meio simulado em RAM para as experiências sem GPU.
- `[ ]` Filas ublk e conclusão única por etiqueta.
- `[ ]` Meio CUDA e travessia real por DMA.
- `[ ]` Endurecimento para pressão de memória e emprego como swap.

## § III. DA PROPOSIÇÃO IMMEDIATA

Lavrar `src/estado_da_requisicao.c` com uma só funcção:

```c
int transicao_da_requisicao_e_valida(enum estado_da_requisicao origem, enum estado_da_requisicao destino);
```

A funcção será pura: não altera estado, não alloca memória e restitue unidade
somente quando a passagem conserva a ordem fixada no tractado technico.

## § IV. DA REGRA DE PASSAGEM

1. Não principiar nova proposição sem prova verde da precedente.
2. Confirmar uma responsabilidade por vez e nunca exceder 50 linhas.
3. Actualizar este livro na mesma confirmação que muda o estado da obra.
4. Registrar comando, resultado e limite conhecido de cada demonstração.
5. Não confundir meta de desempenho com verdade já medida.

## § V. DO LIVRO DE PROVAS

| Confirmação | Proposição | Prova |
|---|---|---|
| `c27eecb` | Proêmio público | revisão integral de 50 linhas |
| `3782567` | Contracto dos estados | C11 com todos os avisos fataes |

**Próximo corollário:** provar exhaustivamente a lei das transições. **Q.E.D.**
