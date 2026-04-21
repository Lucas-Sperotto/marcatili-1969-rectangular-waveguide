# TODO: Exercícios e Projetos de Exploração

Este arquivo lista uma série de exercícios e projetos para expandir a análise do modelo de Marcatili e aprofundar o entendimento do código.

## Análise Paramétrica

- [ ] **Sweep de Razão de Aspecto:** Criar um executável e um caso de teste que fixa `b/A₄` e varre `a/b` de 0.5 a 5.0. Analisar o cruzamento e a separação dos modos `E_y:1:2` e `E_y:2:1`.

- [ ] **Sweep de Contraste de Índice:** Usando um caso base da Fig. 6, gerar uma série de gráficos variando um dos índices externos (e.g., `n4`) para valores cada vez mais próximos de `n1`. Plotar as curvas de dispersão e observar o efeito no confinamento.

- [ ] **Sweep de Comprimento de Onda:** Criar um caso de teste que fixa a geometria e os materiais, mas varre o `wavelength`. Plotar `kz` (não normalizado) em função de `wavelength`. Isso mostrará a dispersão cromática do guia.

## Verificação de Limites e Simetrias

- [ ] **Verificar o Limite de Guia de Lâmina (Slab):**
    1. Executar `solve_single_guide` com uma razão de aspecto muito grande (e.g., `a_over_b: 100.0`).
    2. Executar `solve_slab_guide` com os mesmos parâmetros de altura `b` e materiais.
    3. Comparar os valores de `ky` e `kz` obtidos. A diferença deve ser mínima.

- [ ] **Verificar Degenerescência em Guia Simétrico:**
    1. Criar um caso de teste para um guia perfeitamente simétrico: `a_over_b: 1.0` e `n2 = n3 = n4 = n5`.
    2. Calcular os modos `E_y:1:2` e `E_x:2:1`.
    3. Verificar se as suas constantes de propagação `kz` são idênticas em toda a faixa de varredura.

- [ ] **Quebrar a Simetria:** A partir do caso anterior, introduzir uma pequena perturbação (e.g., `n2` ligeiramente diferente de `n4`). Observar como a degenerescência é removida e as curvas de `kz` se separam.

## Análise Numérica

- [ ] **Plotar o Erro da Aproximação `closed_form`:**
    1. Executar uma simulação da Fig. 6 com os solvers `exact` e `closed_form`.
    2. Criar um script Python que lê o CSV e plota o erro absoluto `abs(kz_exact - kz_closed_form)` e/ou o erro relativo em função de `b/A₄`.
    3. Relacionar o crescimento do erro com os valores de `approximation_checks` no resultado do `closed_form`.

- [ ] **Comparar Métodos de Busca de Raiz (Avançado):**
    1. Implementar um novo buscador de raízes em `src/math/` (e.g., Método da Secante ou Newton-Raphson).
    2. Modificar `SolveSingleGuideExact` para permitir a escolha do algoritmo numérico.
    3. Comparar o número de iterações e a robustez dos diferentes métodos para encontrar as raízes `kx` e `ky`.

## Ambiguidades OCR e Validação Externa

Estes itens dependem de acesso ao scan ou fac-símile editorial do artigo original.

- [ ] **Fig. 8 — eixo horizontal:** Confirmar via OCR se o eixo é $A$ ou $A_4$; congelar a terminologia nos scripts e docs.

- [ ] **Fig. 8 — ramo intermediário:** Confirmar a leitura modal do ramo intermediário (família e índices modais) via scan.

- [ ] **Fig. 10 — família intermediária (1.0 vs 1.6):** Fechar a leitura OCR do parâmetro da curva intermediária; gerar comparação controlada com ambas as hipóteses e congelar com justificativa documentada.

- [ ] **Fig. 10 — coerência rótulo modal:** Fechar a coerência entre o rótulo modal da figura e a referência textual a Eq. (6)/(12) na Seção IV.

- [ ] **Fig. 10 — referência Jones/Goell:** Incorporar dados ou referência rastreável de Jones/Goell para reforçar a validação do acoplador.

- [ ] **Tabela I — fac-símile editorial:** Localizar e comparar com o fac-símile editorial da tabela impressa para confirmar a interpretação `table_entry_interpretation = a_times_n1_over_lambda`.

## Refinamento das Figuras (fac-símile)

Itens de acabamento visual — não bloqueiam conclusões físicas.

- [ ] **Fig. 6 painéis 6c, 6e, 6j:** Revisar cada painel com critérios explícitos de leitura OCR; refinar agrupamento modal do painel `6j`.

- [ ] **Rótulos e bordas:** Alinhar rótulos dos scripts `plot_fig*.py` mais próximos do artigo; revisar limites por painel nas montagens artigo × reprodução.

## Expansão do Acoplador (Seção V)

- [ ] **Guias ligeiramente diferentes:** Expandir `solve_coupler` para cobrir o caso perturbado da Seção V (guias com parâmetros ligeiramente distintos).

- [ ] **Exemplos numéricos rastreáveis:** Reproduzir os exemplos numéricos do texto da Seção V de forma rastreável em `data/input/`.

- [ ] **Documentação da Seção V:** Completar `docs/05_Acoplador direcional construído com guias ligeiramente diferentes.md` com as normalizações e o modelo do caso perturbado.

## Infraestrutura

- [ ] **Migrar JSON compactos:** Migrar casos canônicos remanescentes de formato compacto legado para arrays de objetos JSON; manter strings compactas apenas como compatibilidade.