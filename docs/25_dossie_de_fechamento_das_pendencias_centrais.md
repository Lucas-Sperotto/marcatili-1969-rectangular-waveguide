# 25. Dossiê de fechamento das pendências centrais

Este documento consolida o estado das quatro frentes que mais concentraram discussão no projeto:

- Fig. 8;
- Fig. 10;
- Tabela I;
- ajustes remanescentes da Fig. 6 (`6c`, `6e`, `6j`).

O objetivo aqui não é repetir toda a documentação do repositório, mas registrar:

- qual evidência local foi usada;
- qual decisão operacional foi tomada;
- o que foi fechado em código e teste;
- o que ainda permanece como `TODO OCR` ou dependência externa.

## Fig. 8

### Evidência usada

- transcrição da seção IV e dos resumos em `docs/00.1_figuras.md` e `docs/04_acoplador_direcional.md`;
- caso-base `data/input/reproduce_fig8.json`;
- regressão quantitativa em `tests/regression_checks.cpp`.

### Decisão operacional

- o repositório mantém como família de trabalho os ramos $E^y_{11}$, $E^x_{11}$ e $E^x_{21}$;
- o solver metalizado usa o mesmo critério físico de confinamento contra o meio externo crítico que o restante do núcleo;
- a reprodução atual continua a usar o eixo escrito como $a/A$, com `TODO OCR` explícito para confirmar se o scan final pede $A$ ou $A_4$.

### Fechamentos efetivos

- o comportamento numérico da reprodução canônica está protegido por regressão;
- o falso positivo de `guided` foi removido no núcleo compartilhado;
- a distinção entre `below_cutoff` e `outside_*_domain` vale também para o ramo metalizado.

### O que continua aberto

- confirmação OCR do símbolo do eixo horizontal;
- confirmação OCR da leitura modal final do ramo intermediário.

## Fig. 10

### Evidência usada

- frase transcrita da Seção IV em `docs/04_acoplador_direcional.md`, que associa a figura a Eq. (34) com $k_x$ vindo de Eq. (6) e Eq. (12);
- caso-base `data/input/reproduce_fig10.json`;
- regressão quantitativa no primeiro ponto da curva `a_over_A5=0.5`.

### Decisão operacional

- a implementação continua a seguir o texto da Seção IV, não o rótulo modal isolado do scan;
- o conjunto canônico de curvas permanece `0.5`, `0.75`, `1.0`, `1.5`, `2.0`, `3.0`, `4.0`;
- os casos-base foram migrados para arrays de objetos JSON, preservando compatibilidade com o formato compacto legado.

### Fechamentos efetivos

- o solver do acoplador não aborta mais quando a equação transversal não possui raiz física no intervalo admissível;
- o estado `below_transverse_cutoff` agora é reportado explicitamente;
- `solve_coupler` já reconstrói $A_5$, $|K|$ e $L$ quando `wavelength`, `n1` e `n5` são fornecidos.

### O que continua aberto

- confirmação OCR da família intermediária `1.0` versus `1.6`;
- referência comparativa de Jones/Goell ainda ausente da validação local;
- coerência final entre o rótulo modal da figura e a referência textual a Eq. (6)/(12).

## Tabela I

### Evidência usada

- transcrição documental da Tabela I e dos exemplos associados;
- caso-base `data/input/reproduce_table1.json`;
- saídas `computed_a_normalized` e `computed_dimension_normalized` da implementação;
- regressões quantitativas e checks de `cutoff_status`.

### Decisão operacional

- a interpretação congelada na API é `table_entry_interpretation = a_times_n1_over_lambda`;
- a Tabela I deixa de colapsar todos os resultados num único booleano e passa a distinguir:
  - `found`;
  - `below_search_min`;
  - `above_search_max`;
  - `not_guided_in_search_window`.

### Fechamentos efetivos

- a antiga confusão entre “já guiado no limite inferior” e “cutoff encontrado” foi removida;
- a suíte de regressão cobre explicitamente `found`, `below_search_min` e `above_search_max`;
- a documentação central foi atualizada para tratar a interpretação tabulada como decisão operacional estabilizada.

### O que continua aberto

- fac-símile editorial da tabela impressa;
- revisão histórica adicional só seria útil se trouxesse fonte melhor que o material local já consolidado.

## Fig. 6 (`6c`, `6e`, `6j`)

### Evidência usada

- painéis canônicos de `data/input/fig6/`;
- comparação `closed_form` versus `exact` já presente na reprodução;
- regressão quantitativa em painel assimétrico (`6h`) e estados físicos do solver.

### Decisão operacional

- `6c`, `6e` e `6j` deixam de ser tratados como bloqueio do núcleo;
- o que resta nesses painéis é classificado como ajuste de leitura OCR, agrupamento modal ou acabamento de borda visual.

### Fechamentos efetivos

- o núcleo usa agora o meio externo mais crítico para decidir se um modo está realmente guiado;
- o caminho `exact` distingue `below_cutoff` de erro de domínio;
- a família de testes protege a transição entre pontos guiados e abaixo do cutoff em casos canônicos.

### O que continua aberto

- conferência OCR dos valores destacados em alguns painéis;
- refinamento visual do agrupamento modal de `6j`.

## Resumo final

Depois desta rodada, as pendências centrais ficaram separadas em dois grupos claros:

- **fechadas em código e teste:** semântica física do núcleo, estados do acoplador, status da Tabela I, regressões quantitativas, saída dimensional básica do acoplador;
- **ainda abertas por evidência insuficiente:** OCR de Fig. 8, OCR de Fig. 10, referência de Jones/Goell e acabamento fac-símile.

Isso reduz bastante a área cinzenta do projeto: o que segue em aberto agora já não é mais ambiguidade do núcleo numérico, e sim fechamento documental e comparativo.


<!-- NAV START -->
---

**Navegação:** [Anterior](24_plano_de_melhoria_priorizado.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | Próximo

<!-- NAV END -->
