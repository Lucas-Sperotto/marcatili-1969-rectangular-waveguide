# Output Data

Os arquivos gerados pelos executaveis devem ser gravados aqui.

- `solve_single_guide` ja produz um relatorio `JSON` e uma linha numerica em `CSV`.
- `scripts/run.sh fig6` gera os paineis da Fig. 6 em `data/output/fig6/`, cada um com `JSON`, `CSV` e `PNG`.
- `reproduce_fig7` ja produz um `JSON`, um `CSV` para as retas do nomograma e um `CSV` para as interseccoes de referencia.
- `scripts/run.sh fig7` gera o `PNG` correspondente ao caso-base da Fig. 7.
- `reproduce_fig8` passa a produzir um `JSON` e um `CSV` de sweep para o caso metalizado da Fig. 8.
- `scripts/run.sh fig8` gera o `PNG` correspondente ao caso-base da Fig. 8.
- `reproduce_fig10` passa a produzir um `JSON` e um `CSV` de sweep para a Fig. 10 usando a equacao de acoplamento normalizado da Secao IV.
- `scripts/run.sh fig10` gera o `PNG` correspondente ao caso-base da Fig. 10.
- `reproduce_fig11` passa a produzir um `JSON` e um `CSV` de sweep para a Fig. 11 usando a mesma equacao de acoplamento normalizado, agora com a raiz transversal da Eq. (20).
- `scripts/run.sh fig11` gera o `PNG` correspondente ao caso-base da Fig. 11.
- `reproduce_table1` gera um `JSON`, um `CSV`-resumo por linha da tabela e um `CSV` com os cutoffs pesquisados por modo; nesta etapa, a comparacao principal usa a interpretacao da entrada tabulada como a dimensao `a`.
- O subdiretorio `fig6/` concentra os paineis individuais da Fig. 6.
- Os demais executaveis ainda produzem relatorios placeholder em `JSON`.
- Resultados numericos futuros devem continuar sendo salvos em `CSV` ou `JSON`.
