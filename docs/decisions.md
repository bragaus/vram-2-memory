# CADERNO DAS DECISÕES DO APPARELHO

> Manuscripto de governo da Segunda Jornada, no qual cada escolha declara
> a razão de sua figura e o facto capaz de a fazer regressar ao cálculo.

## § I. DO NASCIMENTO DO SERVIDOR

**Contexto.** Uma instância possue um só dispositivo, uma só GPU e todos os recursos que sustentam as filas. Cumpre decidir quem a faz nascer.

**Alternativas.** Consideraram-se o processo directo em primeiro plano e a unidade transitória entregue a `systemd`.

**Decisão.** `vramdiskd` será iniciado directamente e permanecerá em primeiro plano; `vramdiskctl` governará pela tomada local uma instância já presente.

**Consequência.** O termo, o código de saída e os registros pertencem ao
processo visível. Uma unidade `systemd` futura executará a mesma entrada, sem
crear segundo mecanismo de nascimento.

**Revisão.** A escolha regressará ao gabinete se a prova operacional mostrar
que o processo directo não conserva posse, diagnóstico ou termo singulares.

## § II. DO LIMITE DA MEMÓRIA FIXADA

**Contexto.** A geometria reclama `filas × profundidade × maior_operação` em RAM fixada, além das reservas auxiliares conhecidas antes da publicação.

**Alternativas.** Consideraram-se um número autoral constante e o limite que o
systema já concede ao processo por `RLIMIT_MEMLOCK`.

**Decisão.** A somma exacta deverá caber em `RLIMIT_MEMLOCK`; o apparelho não
elevará esse limite nem escolherá percentagem da RAM em segredo.

**Consequência.** Toda recusa mostrará necessidade e concessão observadas. O
operador mudará o limite exterior somente por acto explícito e documentado.

**Revisão.** A bancada poderá propor um tecto inferior quando demonstrar que a
pressão começa antes do limite exterior. Até lá, nenhum número será inventado.

## § III. DO PRAZO DAS OPERAÇÕES

**Contexto.** Demora finita depende de GPU, PCIe, núcleo, geometria e carga.

**Alternativas.** Consideraram-se um valor presumido e a declaração obrigatória
do prazo enquanto os tectos da bancada ainda não foram medidos.

**Decisão.** O prazo continuará obrigatório na configuração e será medido por
relógio monotónico; não haverá valor ordinário occulto antes da caracterização.

**Consequência.** O operador conhece a fronteira escolhida, e toda expiração
conserva a grandeza que a causou.

**Revisão.** A calibração da Segunda Jornada fixará um valor conservador por classe de bancada, acompanhado da fórmula e dos percentis observados. **Q.E.D.**
