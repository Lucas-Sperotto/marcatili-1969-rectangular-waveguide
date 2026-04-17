# Output Data

Os arquivos gerados pelos executaveis devem ser gravados aqui.

- `solve_single_guide` ja produz um relatorio `JSON` e uma linha numerica em `CSV`.
- `reproduce_fig6` ja produz um relatorio `JSON` e um `CSV` com as curvas do sweep configurado.
- `scripts/plot_fig6.py` gera o `PNG` correspondente a partir do `CSV`.
- `scripts/compare_fig6_article.py` gera montagens lado a lado para comparar o scan do artigo com a reproducao atual.
- `reproduce_fig7` ja produz um `JSON`, um `CSV` para as retas do nomograma e um `CSV` para as interseccoes de referencia.
- `scripts/plot_fig7.py` gera o `PNG` correspondente ao caso-base da Fig. 7.
- `scripts/compare_fig7_article.py` gera uma comparacao lado a lado entre o scan da Fig. 7 e a reproducao atual.
- `reproduce_fig8` passa a produzir um `JSON` e um `CSV` de sweep para o caso metalizado da Fig. 8.
- `scripts/plot_fig8.py` gera o `PNG` correspondente ao caso-base da Fig. 8.
- `scripts/compare_fig8_article.py` gera uma comparacao lado a lado entre o scan da Fig. 8 e a reproducao atual.
- `reproduce_fig10` passa a produzir um `JSON` e um `CSV` de sweep para a Fig. 10 usando a equacao de acoplamento normalizado da Secao IV.
- `scripts/plot_fig10.py` gera o `PNG` correspondente ao caso-base da Fig. 10.
- `scripts/compare_fig10_article.py` gera uma comparacao lado a lado entre o scan da Fig. 10 e a reproducao atual.
- `reproduce_fig11` passa a produzir um `JSON` e um `CSV` de sweep para a Fig. 11 usando a mesma equacao de acoplamento normalizado, agora com a raiz transversal da Eq. (20).
- `scripts/plot_fig11.py` gera o `PNG` correspondente ao caso-base da Fig. 11.
- `scripts/compare_fig11_article.py` gera uma comparacao lado a lado entre o scan da Fig. 11 e a reproducao atual.
- `reproduce_table1` gera um `JSON`, um `CSV`-resumo por linha da tabela e um `CSV` com os cutoffs pesquisados por modo; nesta etapa, a comparacao principal usa a interpretacao da entrada tabulada como a dimensao `a`.
- O subdiretorio `fig6/` concentra paineis individuais e comparacoes com o artigo.
- Os demais executaveis ainda produzem relatorios placeholder em `JSON`.
- Resultados numericos futuros devem continuar sendo salvos em `CSV` ou `JSON`.
