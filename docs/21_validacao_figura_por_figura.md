# 21. Validação figura por figura

As figuras são o coração da validação deste projeto. Este guia explica o que cada uma delas significa fisicamente e qual o status atual de sua reprodução.

## Fig. 6

### O que ela mostra?

Como a constante de propagação axial (`kz`) de diferentes modos varia com a dimensão do guia.

-   **Eixo Horizontal (`b/A₄`):** É a altura do guia (`b`) normalizada por uma escala (`A₄`) que depende do contraste de índice com o substrato. Varrer este eixo é como "engordar" ou "afinar" o guia.
-   **Eixo Vertical (Ordenada Normalizada):** É uma medida de quão bem o modo está confinado.
    -   `1.0`: Confinamento perfeito (toda a energia no núcleo, `kz = k₁`).
    -   `0.0`: Limite do corte (o modo está prestes a "vazar", `kz = k₄`).
-   **As Curvas:** Cada curva representa um modo específico (e.g., $E^y_{11}$).
    -   **Linhas contínuas:** Solver `exact`.
    -   **Linhas tracejadas:** Solver `closed_form`.

-   **O que observar:** A divergência entre as linhas contínuas e tracejadas mostra onde a aproximação de "modo bem guiado" começa a falhar (geralmente perto do corte, na parte de baixo do gráfico).

### Equações relevantes

- Eq. (3)
- Eq. (6) e Eq. (7)
- Eq. (12) a Eq. (16)
- Eq. (17), Eq. (20) e Eq. (24) a Eq. (26)

### Arquivos envolvidos

- `data/input/fig6/*.json`
- `src/physics/fig6.cpp`
- `src/physics/single_guide.cpp`
- `src/physics/slab_guide.cpp`
- `src/io/fig6_io.cpp`
- `scripts/plot_fig6.py`
- `scripts/compare_fig6_article.py`

### Grau atual de concordância

**Bom**, com regressões quantitativas cobrindo um caso canônico e com ajustes finos ainda concentrados em leitura editorial.

Os painéis `6h`, `6i`, `6j` e `6k` já mostram comportamento plausível e comparações úteis. Os casos `6c` e `6e` ainda pedem refinamento de borda vertical e conferência OCR dos rótulos.

### Divergências científicas

- A identificação exata de todos os modos em alguns painéis do artigo original é difícil devido à qualidade do scan, mas o comportamento geral é bem reproduzido.
- O agrupamento modal em `6j` ainda merece ajuste fino editorial, não um bloqueio numérico do solver.

### Divergências editoriais

- ausência de rótulos sobre as curvas;
- legenda em caixa;
- falta de esquemas internos de seção transversal;
- falta de montagem artigo $\times$ reprodução para todos os painéis;
- diferença de ocupação de borda em alguns casos.

### Prioridade de correção

**Média-alta**, porque a física já está boa, mas a figura é central para o projeto e ainda merece acabamento científico-editorial.

## Fig. 7

### O que ela mostra?

Uma ferramenta gráfica para projetar um guia monomodo.

-   **Eixos (`X` e `Y`):** São variáveis complexas que combinam geometria e propriedades dos materiais.
-   **As Curvas:**
    -   **Linhas retas contínuas:** Representam as condições de corte para cada modo superior (e.g., $E^y_{12}$, $E^y_{21}$).
    -   **Linha pontilhada:** Representa uma família de guias com uma razão de aspecto e materiais específicos.
-   **Como usar:** Para garantir operação monomodo, o ponto de projeto do seu guia (um ponto no plano X-Y) deve estar dentro do triângulo formado pelas linhas dos primeiros modos superiores e os eixos.

### Equações relevantes

- Eq. (27) a Eq. (30)
- uso implícito do bloco fechado derivado de Eq. (12) a Eq. (16)

### Arquivos envolvidos

- `data/input/reproduce_fig7.json`
- `src/physics/fig7.cpp`
- `src/io/fig7_io.cpp`
- `scripts/plot_fig7.py`
- `scripts/compare_fig7_article.py`

### Grau atual de concordância

**Bom** numericamente e **moderado** editorialmente.

O comportamento geométrico do nomograma está consistente e o caso destacado com $C=25$ cai em região plausível do scan.

### Divergências científicas

- existe um `TODO OCR` sobre a coerência entre o rótulo gráfico do modo destacado e a frase transcrita no texto.

### Divergências editoriais

- falta a riqueza de anotações do quadro original;
- os pontos de interseção são úteis pedagogicamente, mas não existem no fac-símile;
- a composição ainda está mais “limpa” do que o impresso.

### Prioridade de correção

**Média**, porque a leitura física principal já está satisfatória.

## Fig. 8

### O que ela mostra?

As curvas de dispersão para um guia que tem uma de suas faces coberta por metal.

-   **O que observar:** A presença do metal quebra a simetria e altera drasticamente quais modos podem se propagar. O artigo sugere que isso pode ser usado para forçar a operação em um único modo e polarização ($E^y_{11}$).

### Equações relevantes

- adaptação do bloco Eq. (6), Eq. (7), Eq. (12), Eq. (13)
- adaptação do bloco Eq. (20), Eq. (21), Eq. (22), Eq. (23)
- ligação conceitual com o Apêndice A

### Arquivos envolvidos

- `data/input/reproduce_fig8.json`
- `src/physics/metal_guide.cpp`
- `src/physics/fig8.cpp`
- `src/io/fig8_io.cpp`
- `scripts/plot_fig8.py`
- `scripts/compare_fig8_article.py`

### Grau atual de concordância

**Moderado para bom**, com a física geral convincente e uma hipótese modal de trabalho agora estabilizada em código e teste.

### Divergências científicas

- **Ambiguidade Principal:** A identificação do "ramo intermediário" no scan original do artigo ainda é uma hipótese aberta. A implementação atual adota a família de trabalho $E^y_{11}$, $E^x_{11}$ e $E^x_{21}$, mas a questão não está cientificamente encerrada.
- o símbolo final do eixo horizontal ainda merece conferência: $A$ versus $A_4$.

### Divergências editoriais

- falta o esquema da interface metalizada dentro do quadro;
- a composição ainda não imita o scan;
- a legenda ainda substitui rótulos internos.

### Prioridade de correção

**Alta**, porque aqui o principal resíduo aberto ainda é científico, não apenas visual.

## Fig. 10

### O que ela mostra?

Quão forte é o acoplamento entre dois guias paralelos para o modo rotulado como $E^x_{11}$ na Seção IV.

-   **Eixo Horizontal (`c/a`):** A separação entre os guias (`c`) normalizada pela largura (`a`).
-   **Eixo Vertical (Acoplamento Normalizado):** Uma medida da força da interação. Um valor maior significa acoplamento mais forte (transferência de potência mais rápida).
-   **As Curvas:** Cada curva representa uma "família" de guias com uma geometria transversal específica (parâmetro `a/A₅`).
-   **O que observar:** O acoplamento cai *exponencialmente* com a separação, pois depende do campo evanescente.

### Equações relevantes

- Eq. (33)
- Eq. (34)
- Eq. (6) no caso `exact`
- Eq. (12) no caso `closed_form`

### Arquivos envolvidos

- `data/input/reproduce_fig10.json`
- `src/physics/coupler.cpp`
- `src/physics/fig10.cpp`
- `src/io/fig10_io.cpp`
- `scripts/plot_fig10.py`
- `scripts/compare_fig10_article.py`

### Grau atual de concordância

**Bom** no comportamento global, com regressão quantitativa no caso-base e uma pendência OCR ainda relevante.

### Divergências científicas

- **Ambiguidade Principal:** A identificação da família de curvas intermediária (`1.0` vs. `1.6`) no scan original ainda não foi encerrada por fonte melhor. O repositório mantém a família `1.0` como baseline de trabalho, porque ela preserva o conjunto monotônico documentado nos casos-base.
- O texto da Seção IV cita Eq. (6) e Eq. (12) (da família $E^y_{pq}$), embora o título da figura mencione modos $E^x_{pq}$. A implementação segue o texto.
- a referência de Jones ainda não foi incorporada.

### Divergências editoriais

- legenda em caixa;
- ausência do esquema interno do acoplador;
- estilo gráfico mais limpo do que o scan.

### Prioridade de correção

**Alta**, porque a ambiguidade da família intermediária muda a leitura científica da figura.

## Fig. 11

### O que ela mostra?

O mesmo tipo de curva da Fig. 10, agora para o modo rotulado como $E^y_{11}$ na Seção IV.

-   **Diferença para a Fig. 10:** o caso-base do repositório segue explicitamente a instrução textual da Seção IV e usa a raiz transversal de Eq. (20) para `kx`.
-   **O que observar:** Para $n_1/n_5$ próximo de 1, os modos $E^x_{11}$ e $E^y_{11}$ se aproximam da degenerescência, e as curvas da Fig. 11 se aproximam das da Fig. 10.

### Equações relevantes

- Eq. (34)
- Eq. (20) para a raiz transversal exata
- Eq. (17), Eq. (18) e Eq. (19) como pano de fundo modal da família $E^x_{pq}$ no artigo

### Arquivos envolvidos

- `data/input/reproduce_fig11.json`
- `src/physics/coupler.cpp`
- `src/physics/fig11.cpp`
- `src/io/fig11_io.cpp`
- `scripts/plot_fig11.py`
- `scripts/compare_fig11_article.py`

### Grau atual de concordância

**Bom**.

O caso-base captura bem:

- as duas famílias de legenda $n_1/n_5=1.5$ e $n_1/n_5=1.1$;
- a tendência de aproximação à Fig. 10 quando o contraste externo diminui.

### Divergências científicas

- menos graves que em Fig. 10;
- o principal ponto aberto é ainda a consistência global do bloco do acoplador, e não uma ambiguidade central desta figura em si.

### Divergências editoriais

- ainda falta o esquema do acoplador dentro do quadro;
- a anotação de curvas não imita o artigo;
- a composição visual ainda é moderna demais.

### Prioridade de correção

**Média**, porque a física-base está razoavelmente estabilizada.

## Tabela I

### O que ela mostra?

As maiores dimensões que um guia pode ter para suportar *apenas* os modos fundamentais ($E^x_{11}$ e $E^y_{11}$).

-   **Como ler:** Para uma dada geometria (`a=b`, `a=2b`, etc.) e contraste de índice (`n₁/n₄`), a tabela fornece um número. Esse número, multiplicado por `λ/n₁`, dá a dimensão física limite.

### Equações relevantes

- Eq. (3), Eq. (6), Eq. (7), Eq. (12) a Eq. (16)
- Eq. (17), Eq. (20), Eq. (21), Eq. (24) a Eq. (26)
- uso operacional das curvas de Fig. 6 como origem conceitual da tabela

### Arquivos envolvidos

- `data/input/reproduce_table1.json`
- `src/physics/table1.cpp`
- `src/physics/single_guide.cpp`
- `src/io/table1_io.cpp`
- `scripts/run/check_reproduction.sh`

### Grau atual de concordância

**Bom**, porque a mecânica numérica agora distingue explicitamente cutoff encontrado, cutoff abaixo da faixa pesquisada e cutoff acima da janela.

### Divergências científicas

- A interpretação operacional foi congelada como `a_times_n1_over_lambda`, consistente com a comparação entre o artigo, os casos-base e os campos `computed_a_normalized` da implementação.
- O ponto científico que ainda resta aqui é menos “qual grandeza está sendo tabulada” e mais o fechamento editorial do fac-símile final da tabela.

### Divergências editoriais

- a tabela do artigo é compacta e direta, enquanto a reprodução atual é uma saída analítica rica em `JSON` e `CSV`;
- isso é bom para auditoria, mas ainda não é um fac-símile visual da tabela impressa.

### Prioridade de correção

**Média-alta**, porque a leitura física central está estabilizada, mas o acabamento editorial da tabela ainda não está fechado.

## Resumo de prioridades

Se a correção for priorizada por impacto científico, a ordem recomendada é:

1. Fig. 8
2. Fig. 10
3. Fig. 6
4. Fig. 7
5. Fig. 11
6. Tabela I

Se a priorização for por acabamento editorial, a ordem muda para:

1. Fig. 6
2. Fig. 10
3. Fig. 11
4. Fig. 7
5. Fig. 8
6. Tabela I


<!-- NAV START -->
---

**Navegação:** [Anterior](20_auditoria_tecnica_do_modelo.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](22_matriz_artigo_para_codigo.md)

<!-- NAV END -->
