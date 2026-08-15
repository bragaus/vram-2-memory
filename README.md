<div align="center">

# { vram-2-memory }
### TRACTADO DA MEMÓRIA GEOMÉTRICA OCCULTA NA MACHINA DE VÍDEO

*Da lavra do Doutor Braga Us, Professor de Sciencias Mathematicas, que no Anno da Graça de MDCCCXCVIII buscou RAM onde o fabricante não a poz.*

</div>

> **ADVERTÊNCIA PRELIMINAR.** Este apparelho existe ainda no papel e no cálculo.
> Quando funccionar, a morte do servidor, da machina ou da GPU poderá extinguir
> cada página confiada à VRAM; não se depositem aqui cousas que não possam renascer.

## § I. PROÊMIO — da grandeza que se deseja alcançar

Seja dada uma machina cuja RAM, grandeza finita e dispendiosa, já não baste;
ao lado della repousa uma GPU com VRAM ociosa, reservatório veloz porém alheio
ao domínio ordinário da CPU. Cumpre fazer d'esta sobra um instrumento útil.

Não se pretende baptisar VRAM como RAM do systema. Tal proposição seria falsa:
a barra PCIe não possue a coherência nem a latência da memória principal.
Erigir-se-á, pois, um dispositivo de blocos volátil para `swap` e dados refazíveis.

## § II. DO CAMINHO — por onde cada octeto ha-de peregrinar

```text
memória virtual
  → zswap
  → camada de blocos e ublk
  → servidor lavrado em C11
  → motores de cópia CUDA
  → VRAM
```

**Postulado central.** Todo octeto atravessará DMA. A CPU jámais fará leitura
ou escripta pela BAR; ReBAR, DAX, NUMA e FUSE não pertencem a esta demonstração.

## § III. DOS AXIOMAS — condições para que a obra permaneça honesta

- o meio é volátil e recebe somente dados que possam ser reconstruídos;
- cada requisição conclue uma vez, ou falla dentro de prazo conhecido;
- toda memória do caminho crítico se prepara antes de publicar o dispositivo;
- a prova começa sem GPU, por meio simulado, e termina sob `fio` com verificação.

## § IV. DO ESTADO PRESENTE — onde repousa a penna

A pesquisa e a especificação acham-se concluídas; o código ainda não nasceu.
Primeiro demonstrar-se-ão a integridade byte a byte e o núcleo ublk simulado;
depois virão CUDA, pressão de memória, medições e o delicado offício de `swap`.
**Q.E.D.**
