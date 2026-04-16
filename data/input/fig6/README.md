# Figure 6 Inputs

Este diretorio guarda casos de entrada para paineis finitos da Fig. 6.

- `SG-006a`, `SG-006b`, `SG-006c`: guia totalmente embebido em um unico dielétrico com contraste fraco.
- `SG-006d`: limite de lamina simetrica, resolvido como problema unidimensional no eixo $y$.
- `SG-006e`, `SG-006f`, `SG-006g`: guia totalmente embebido em um unico dielétrico com contraste mais forte.
- `SG-006h`, `SG-006i`, `SG-006j`: guia assimetrico do tipo Fig. 1a, usando a transcricao atual do repositorio com `n2 = n1 / 1.5` e `n3 = n4 = n5 ≈ n1 / 1.01`.
- `SG-006k`: limite de lamina assimetrica, resolvido como problema unidimensional no eixo $y$.

Observacoes:

- Os paineis `6d` e `6k` usam `geometry_model = "slab"` e implementam o limite $a \to \infty$ com $k_x = 0$.
- O painel `6d` inclui variantes de material para comparar o caso de contraste quase nulo, `n_4 = n_1 / 1.1` e `n_4 = n_1 / 1.5`, como sugerido pela leitura atual da figura.
- Nos casos de lamina, os campos variam apenas em $y$; por isso os modos sao cadastrados com `p=1` apenas para manter compatibilidade com a nomenclatura do restante do repositorio.
- Os labels graficos dos casos de lamina sao apresentados como $E^x_{0q}$ e $E^y_{0q}$, para refletir melhor a notacao do artigo nesses limites.
- Os paineis `6h` a `6j` foram configurados com a melhor leitura atual do artigo; se a revisao do OCR refinar quais interfaces usam ar ou dielétrico fraco, estes arquivos devem ser ajustados.
- Os paineis `6h` a `6j` usam uma faixa mais baixa de `b/A4` e uma malha mais densa para destacar melhor o cutoff finito esperado no caso assimetrico.
- No painel `6j`, a interface externa foi deixada com contraste muito fraco para aproximar melhor a posicao observada do par $E_{12}$ no scan atual; essa escolha ainda pode precisar de ajuste fino.
- Cada arquivo usa `solver_models = ["closed_form", "exact"]` para permitir comparacao direta.
