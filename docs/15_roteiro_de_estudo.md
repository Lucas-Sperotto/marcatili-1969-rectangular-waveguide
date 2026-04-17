# 15. Roteiro de Estudo do Repositório

Este documento oferece um caminho estruturado para entender o projeto, desde a física do artigo de Marcatili até a implementação em C++ e a geração dos gráficos.

## Filosofia da Documentação

A documentação deste projeto é organizada em uma trilha principal, com numeração crescente, para guiar o leitor do geral ao específico:

-   **`docs/0*`:** Tradução e comentários diretos sobre o artigo original.
-   **`docs/10-*`:** Documentos de arquitetura e fluxo, explicando *como* o repositório está organizado e *como* o código implementa a física.
-   **`docs/20-*`:** Documentos de auditoria e validação, detalhando o status atual da reprodução, os riscos e as pendências.

## Ordem de Leitura Sugerida para Iniciantes

Se você está começando, a melhor abordagem é seguir esta sequência:

1.  **Visão Geral:** Comece pelo `README.md` principal para entender o propósito do projeto.
2.  **Fluxo de Execução:** Entenda como as peças se conectam lendo o `docs/10_fluxo_geral_do_repositorio.md` e olhando os diagramas em `docs/14_diagramas_de_fluxo_e_sequencia.md`.
3.  **A Física:** Com a estrutura em mente, mergulhe na física lendo os documentos `docs/01_...` a `docs/08_...`, que são a tradução comentada do artigo.
4.  **Código vs. Artigo:** Finalmente, conecte a teoria ao código com o `docs/12_trilha_equacoes_para_codigo.md`.
5.  **Glossário:** Mantenha esta seção aberta para consulta dos termos técnicos.

## Roteiro de Estudo Detalhado

### Passo 1: A Física do Problema

-   **O que fazer:** Leia os documentos na pasta `docs/` que traduzem o artigo (arquivos `01` a `08`).
-   **Objetivo:** Entender a aproximação de Marcatili, a separação do problema em duas famílias de modos ($E^y_{pq}$ e $E^x_{pq}$) e a formulação do guia único e do acoplador.
-   **Foco:** Preste atenção em como as equações transcendentais (Eq. 6, 7, 20, 21) surgem das condições de contorno.

### Passo 2: O Guia de Onda Único

-   **O que fazer:** Estude a diferença entre os solvers `closed_form` e `exact` lendo o `docs/11_closed_form_vs_exact.md`. Em seguida, explore o código em `src/physics/single_guide.cpp`.
-   **Objetivo:** Compreender como as equações do artigo são implementadas. Veja como o solver `exact` usa um buscador de raízes (`src/math/root_finding.cpp`) e como o `closed_form` aplica as fórmulas algébricas diretas.

### Passo 3: O Acoplador Direcional

-   **O que fazer:** Leia a documentação sobre o acoplador (`docs/04_...` e `docs/07_...`) e examine o código em `src/physics/coupler.cpp`.
-   **Objetivo:** Entender como o coeficiente de acoplamento `K` é calculado a partir das propriedades do guia único, conforme a Eq. (34) do artigo.

### Passo 4: A Reprodução das Figuras

-   **O que fazer:** Explore os executáveis em `src/apps/` (e.g., `reproduce_fig6.cpp`) e os arquivos de entrada em `data/input/`.
-   **Objetivo:** Ver como os "solvers" da camada de física são orquestrados para varrer parâmetros e gerar os dados brutos (`.csv`) que correspondem a cada figura do artigo.

### Passo 5: A Visualização dos Resultados

-   **O que fazer:** Leia os scripts Python em `scripts/` (e.g., `plot_fig6.py`).
-   **Objetivo:** Entender a etapa final do fluxo: como os dados numéricos são lidos dos arquivos `.csv` e transformados nos gráficos `.png` que você vê em `data/output/`.

## Como ler as figuras reproduzidas

Esta leitura rápida ajuda a não confundir papel físico da figura com acabamento editorial.

### Fig. 6

-   **O que observar:** como $k_z$ normalizado muda com $b/A_4$ para diferentes modos.
-   **Leitura física:** quando a curva desce em direção a `0`, o modo se aproxima do cutoff; quando sobe para perto de `1`, o confinamento é mais forte.

### Fig. 7

-   **O que observar:** o nomograma como ferramenta geométrica de projeto.
-   **Leitura física:** as interseções entre retas modais e a linha $Y=CX$ indicam combinações geométricas e dielétricas compatíveis com o regime desejado.

### Fig. 8

-   **O que observar:** como a interface metalizada altera a hierarquia modal e pode favorecer um único modo/polarização.
-   **Leitura física:** esta figura continua com uma ambiguidade OCR aberta no ramo intermediário; ela deve ser lida como caso em revisão, não como questão encerrada.

### Fig. 10 e Fig. 11

-   **O que observar:** o acoplamento cai exponencialmente com a separação normalizada $c/a$.
-   **Leitura física:** isso acontece porque o acoplamento depende da sobreposição do campo evanescente entre os dois guias.
-   **Cuidado:** a Fig. 10 ainda preserva explicitamente a ambiguidade `1.0` versus `1.6`, além da tensão entre o rótulo modal e a referência textual a Eq. (6)/(12).

### Tabela I

-   **O que observar:** o primeiro cutoff de modo superior define a fronteira prática de operação monomodo.
-   **Cuidado:** a interpretação final da grandeza tabulada continua aberta e deve ser lida junto com `table_entry_interpretation` no caso-base atual.

Para o estado técnico detalhado de cada artefato, veja `docs/21_validacao_figura_por_figura.md`.

## Glossário Físico e Numérico

Este glossário explica os termos técnicos mais importantes do projeto.

---

### Índice de Refração (`n`)
-   **O que é:** Uma medida de quão mais devagar a luz viaja em um material em comparação com o vácuo. Um índice de refração maior significa que a luz é mais "lenta".
-   **No artigo:** `n1` é o índice do núcleo do guia, enquanto `n2` a `n5` são os índices dos meios circundantes. O guiamento de luz só ocorre se `n1` for maior que os índices vizinhos.

### Contraste Fraco
-   **O que é:** A suposição de que os índices de refração do núcleo e do revestimento são muito próximos.
-   **No artigo:** Formalizado como `(n1/n_v) - 1 << 1`. Essa aproximação simplifica as equações de Maxwell, permitindo a separação em duas famílias de modos (quase-TEM).

### Modo Guiado
-   **O que é:** Um padrão de campo eletromagnético que se propaga ao longo do guia sem escapar, confinado por reflexão interna total.
-   **No código:** Um resultado é considerado `guided` se sua constante de propagação axial `kz` for maior que a do meio externo e menor ou igual à do núcleo.

### Cutoff (Corte)
-   **O que é:** A condição limite (de frequência ou dimensão) na qual um modo deixa de ser guiado. Abaixo do corte, a energia "vaza" para o revestimento.
-   **No código:** Identificado quando a condição de guiado não é mais satisfeita. O status do resultado muda para `below_cutoff`.

### Campo Evanescente
-   **O que é:** Fora do núcleo do guia, o campo não se anula abruptamente, mas decai exponencialmente. Esse campo "vazado" é o campo evanescente.
-   **No artigo:** É a existência desse campo que permite o acoplamento entre guias próximos.

### Profundidade de Penetração (`ξ`, `η`)
-   **O que é:** A distância na qual a amplitude do campo evanescente cai para `1/e` (cerca de 37%) de seu valor na interface. Uma profundidade de penetração pequena significa um modo fortemente confinado.
-   **No artigo:** Definida nas Eq. (8), (9), (18) e (19).
-   **No código:** Calculada em `src/math/waveguide_math.cpp` e usada nos solvers.

### Constantes de Propagação (`kx`, `ky`, `kz`)
-   **O que são:** Componentes do vetor de onda (`k`) dentro do guia.
    -   `kx` e `ky` (transversais): Descrevem como o campo varia nas direções `x` e `y`. Seus valores são "quantizados" pelas dimensões do guia.
    -   `kz` (axial): Descreve como o modo se propaga ao longo do eixo `z`. É a principal grandeza de interesse para entender a dispersão.
-   **No artigo:** Ligadas pela Eq. (3): `k_z² = k_1² - k_x² - k_y²`. O desafio é encontrar `kx` e `ky`.

### Escala Característica (`A₂`, `A₃`, `A₄`, `A₅`)
-   **O que é:** Uma dimensão normalizada que depende apenas do comprimento de onda e do contraste de índices entre o núcleo e um meio externo.
-   **No artigo:** Definida na Eq. (10). Ela serve como uma régua natural para medir as dimensões do guia. Por exemplo, o eixo horizontal da Fig. 6 é `b/A₄`.
-   **No código:** Calculada pela função `ComputeA` em `src/math/waveguide_math.cpp`.

### Solver `closed_form` (Forma Fechada)
-   **O que é:** Um solver que usa as **fórmulas algébricas aproximadas** do artigo (e.g., Eq. 12 e 13).
-   **Como funciona:** Baseia-se na aproximação de "modo bem guiado", que simplifica as equações transcendentais.
-   **Vantagem:** Extremamente rápido.
-   **Desvantagem:** Menos preciso perto da condição de corte.
-   **Para saber mais:** `docs/11_closed_form_vs_exact.md`.

### Solver `exact` (Exato)
-   **O que é:** Um solver que **resolve numericamente as equações transcendentais** do modelo de Marcatili (e.g., Eq. 6 e 7).
-   **Como funciona:** Usa um algoritmo de busca de raízes (bisseção) para encontrar `kx` e `ky` com alta precisão.
-   **Vantagem:** Preciso em toda a faixa de validade do modelo de Marcatili.
-   **Desvantagem:** Computacionalmente mais lento que o `closed_form`.
-   **Importante:** "Exato" refere-se à solução do *modelo simplificado* de Marcatili, não das equações de Maxwell completas para a geometria real.
-   **Para saber mais:** `docs/11_closed_form_vs_exact.md`.

### Normalização de Curvas
-   **O que é:** O processo de reescalar as variáveis de um gráfico para que as curvas se tornem independentes de algumas unidades ou parâmetros, revelando um comportamento universal.
-   **No artigo:**
    -   Na Fig. 6, o eixo `y` é `(k_z² - k_4²) / (k_1² - k_4²)`. Este valor vai de 0 (no corte) a 1 (confinamento total).
    -   Na Fig. 10, o eixo `y` é um coeficiente de acoplamento normalizado.
-   **Objetivo:** Permitir que um único gráfico descreva uma vasta gama de guias com diferentes materiais e dimensões.


<!-- NAV START -->
---

**Navegação:** [Anterior](14_diagramas_de_fluxo_e_sequencia.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](20_auditoria_tecnica_do_modelo.md)

<!-- NAV END -->
