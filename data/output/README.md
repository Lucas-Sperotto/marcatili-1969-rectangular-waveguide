# Output Data

Os arquivos gerados pelos executaveis devem ser gravados aqui.

- `solve_single_guide` ja produz um relatorio `JSON` e uma linha numerica em `CSV`.
- `reproduce_fig6` ja produz um relatorio `JSON` e um `CSV` com as curvas do sweep configurado.
- `scripts/plot_fig6.py` gera o `PNG` correspondente a partir do `CSV`.
- `scripts/compare_fig6_article.py` gera montagens lado a lado para comparar o scan do artigo com a reproducao atual.
- `reproduce_fig7` ja produz um `JSON`, um `CSV` para as retas do nomograma e um `CSV` para as interseccoes de referencia.
- `scripts/plot_fig7.py` gera o `PNG` correspondente ao caso-base da Fig. 7.
- O subdiretorio `fig6/` concentra paineis individuais e comparacoes com o artigo.
- Os demais executaveis ainda produzem relatorios placeholder em `JSON`.
- Resultados numericos futuros devem continuar sendo salvos em `CSV` ou `JSON`.
