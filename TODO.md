# TODO: Exercícios e Projetos de Exploração

Este arquivo lista uma série de exercícios e projetos para expandir a análise do modelo de Marcatili e aprofundar o entendimento do código.

## Análise Paramétrica

- [x] **Sweep de Razão de Aspecto:** Executável `reproduce_sweep_aspect` criado (maio/2026). CSV em `data/output/sweep_aspect.csv`, 91 pontos para `E_y:1:2` e `E_y:2:1`.

- [x] **Plot do Sweep de Razão de Aspecto (N1):** `scripts/plot_sweep_aspect.py` — PNG em `data/output/sweep_aspect.png` (maio/2026).

- [x] **Sweep de Contraste de Índice (A2):** Executável `reproduce_sweep_index` criado (maio/2026). CSV em `data/output/sweep_index.csv`; plot em `data/output/sweep_index.png` via `scripts/plot_sweep_index.py`.

- [x] **Sweep de Comprimento de Onda (A3):** Executável `reproduce_sweep_wavelength` criado (maio/2026). CSV em `data/output/sweep_wavelength.csv`; plot em `data/output/sweep_wavelength.png` via `scripts/plot_sweep_wavelength.py`.

## Verificação de Limites e Simetrias

- [x] **Verificar o Limite de Guia de Lâmina (Slab) (N8):** `solve_slab_guide` criado (maio/2026) a partir de `src/physics/slab_guide.cpp` existente. `scripts/verify_slab_limit.py` rodou com sucesso; kz_relative_error_max ≈ 1.98e-06.

- [x] **Verificar Degenerescência em Guia Simétrico (A5):** `data/input/test_degeneracy.json` criado (maio/2026). Resultado: max |Δkz| = 0 (degenerescência confirmada).

- [x] **Quebrar a Simetria (A6):** `data/input/test_symmetry_break.json` com `n2=1.452` (maio/2026). Resultado: max |Δkz| = 149.165 rad/m (separação confirmada). Plot em `data/output/degeneracy_test.png`.

## Análise Numérica

- [x] **Plotar o Erro da Aproximação `closed_form`:** `scripts/plot_error_closed_form.py` (maio/2026). PNG em `data/output/error_closed_form.png`.

- [x] **Comparar Métodos de Busca de Raiz (A8/C2-C4):** Secante, Newton, Falsa Posição e Ponto Fixo implementados em `src/math/root_finding.cpp`; `solve_single_guide` aceita `solver_algorithm` para bisseção, secante, Newton e falsa posição. Plot em `data/output/root_method_comparison.png`.

## Ambiguidades OCR e Validação Externa

Estes itens dependem de acesso ao scan ou fac-símile editorial do artigo original.

- [ ] **Fig. 8 — eixo horizontal:** Análise Gemini (maio/2026) confirmou eixo como `a/A` — rótulo visual provavelmente `A` (sem subscrito), mas quantidade física é `A₄`. Pendente: confirmar via scan de maior qualidade.

- [ ] **Fig. 8 — ramo intermediário:** Análise Gemini (maio/2026) confirmou hipótese de trabalho `E^x₁₁` como ramo intermediário. Não definitivo — aguarda scan de maior qualidade.

- [x] **Fig. 10 — família intermediária e testes numéricos (N6):** `data/input/reproduce_fig10_test16.json` criado com hipótese 1.6 (maio/2026). Comparação visual em `data/output/reproduce_fig10_compare_test16.png`.

- [x] **Fig. 10 — coerência rótulo modal e teste Eq. (20) (N7):** `data/input/reproduce_fig10_eq20.json` criado (maio/2026). `fig10.cpp/hpp/io` atualizados para suportar `transverse_equation: "eq20"`. Comparação em `data/output/reproduce_fig10_compare_test16_eq20.png`.

- [x] **Fig. 10 — referência Jones/Goell (B5):** Análise Gemini (maio/2026): Jones=[5] valida o acoplador (Fig. 10); Goell=[4] valida o guia único (Fig. 6). Citação `[5]` adicionada em `docs/04_acoplador_direcional.md`.

- [x] **Tabela I — fac-símile editorial:** Análise Gemini (maio/2026) confirmou interpretação `table_entry_interpretation = a_times_n1_over_lambda`. Docs consistentes.

## Refinamento das Figuras (fac-símile)

Itens de acabamento visual — não bloqueiam conclusões físicas.

- [ ] **Fig. 6 painéis 6c, 6e, 6j (B7):** Análise Gemini (maio/2026): 6c e 6e estão alinhados. Pendência científica em 6j — agrupamento modal intermediário levemente deslocado; requer revisão dos parâmetros de entrada. Pendência editorial: rótulos sobre curvas (todos os painéis).

- [ ] **Rótulos e bordas (B8):** Análise Gemini (maio/2026): divergências são principalmente editoriais (anotações internas, esquemas geométricos, caixas de legenda). Rótulos de eixo estão conceitualmente corretos em Fig. 7, 8, 10, 11.

## Regressão da Fig. 6

- [x] **Fix do parser fig6_io.cpp (C1):** `ParseFigure6Config()` em `src/io/fig6_io.cpp` atualizado pelo CODEX (maio/2026) para detectar raiz array `[{...}]` e extrair o primeiro objeto antes de parsear campos. Testes `reproduce_fig6_smoke` e `reproduce_fig6_slab_smoke` passando.

## Expansão do Acoplador (Seção V)

- [x] **Guias ligeiramente diferentes:** `solve_coupler` expandido para caso perturbado (maio/2026). `src/physics/coupler.cpp`; `data/input/solve_coupler_perturbed.json`.

- [x] **Exemplos numéricos rastreáveis (A10):** `solve_coupler_perturbed_ex1.json` (assimetria de altura h/b=1%, δ≈6299 rad/m) e `solve_coupler_perturbed_ex2.json` (assimetria de índice Δn₁=0.005, δ≈−14348 rad/m) — maio/2026.

- [x] **Documentação da Seção V:** `docs/05_Acoplador...` atualizado com teoria formal de modos acoplados (δ, F, L_c) — maio/2026.

## Infraestrutura

- [x] **Migrar JSON compactos:** 11 arquivos em `data/input/fig6/` convertidos (maio/2026). Fix C1 aplicado — `reproduce_fig6` funcionando.

## Extensão de Métodos Numéricos (Zeros de Funções)

Referência: Zeros.c, Prof. Sperotto, UNEMAT 2014. Objetivo: implementar os 5 métodos clássicos e comparar convergência aplicada à equação transcendental do guia de onda.

- [x] **Implementar Newton, Falsa Posição e Ponto Fixo (C2):** `newton_fd`, `false_position` e `fixed_point` adicionados a `include/marcatili/math/root_finding.hpp` e `src/math/root_finding.cpp` (maio/2026). Estilo idêntico ao `secant()` com `int* iter_count = nullptr`. 15/15 testes passando.

- [x] **Integrar Newton e Falsa Posição no single_guide (C3):** `SolveExactRoot()` em `src/physics/single_guide.cpp` extendido para `"newton"` (guess = midpoint do bracket) e `"false_position"` (mesmo bracket da bisseção) — maio/2026. Newton = 7 iter, falsa posição = 16 iter. Arquivos `data/input/test_root_newton.json` e `data/input/test_root_false_position.json` criados.

- [x] **Comparação completa de 4 métodos na eq. transcendental (C4):** `scripts/compare_root_methods.py` atualizado (maio/2026). Resultado em `data/output/root_method_comparison.png`. Iterações para kx/ky: bisseção=43/42, secante=5/5, Newton=7/7, falsa posição=16/16.

- [x] **Script educacional com f(x) = x³+4x²-10 (C5):** `scripts/compare_zeros_reference.py` criado (maio/2026). Implementa os 5 métodos em Python puro na equação do Zeros.c. Resultado em `data/output/zeros_reference_comparison.png`. Iterações: bisseção=17, ponto fixo=17, Newton=4, secante=7, falsa posição=9.

## Validação Externa (Koshiba 1992)

- [ ] **Comparação Marcatili vs Koshiba Fig. 5 (D1):** `scripts/koshiba_fig5_reference.py` e `data/output/koshiba_fig5/` criados (maio/2026) — gera curvas b(V) do modelo de Marcatili para os casos low-contrast (n₁=1.05, n_clad=1.0) e high-contrast (n₁=1.5, n_clad=1.0). **Pendente:** script de plot que sobreponha dados de Marcatili com pontos digitalizados da Fig. 5 de Koshiba & Inoue (1992) para validação visual quantitativa.
