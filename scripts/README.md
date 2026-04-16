# Scripts

Os scripts Python deste diretorio devem cuidar apenas de tarefas externas ao nucleo C++.

- Geracao de graficos a partir de arquivos `CSV` e `JSON`.
- Conversao e comparacao de dados de referencia.
- Ferramentas auxiliares de reproducao.

## Scripts atuais

- `plot_fig6.py`: plota um painel de Fig. 6 a partir do `CSV` gerado por `reproduce_fig6`.
- `compare_fig6_article.py`: monta uma comparacao lado a lado entre o scan do artigo e o `PNG` gerado para os paineis atualmente reproduzidos.
- `plot_fig7.py`: plota o nomograma da Fig. 7 a partir dos `CSV` de linhas e interseccoes gerados por `reproduce_fig7`.
- `compare_fig7_article.py`: monta uma comparacao lado a lado entre o scan da Fig. 7 e a reproducao atual do nomograma.
- `run/reproduce_fig6_panels.sh`: gera JSON, CSV e PNG para os paineis finitos de Fig. 6 atualmente configurados.
- `run/build_fig6_article_comparisons.sh`: atualiza os paineis numericos disponiveis e gera comparacoes `artigo x reproducao`, incluindo os limites de lamina `6d` e `6k`.
- `run/reproduce_fig7_nomogram.sh`: gera os artefatos numericos e o `PNG` do caso-base atualmente configurado para a Fig. 7.
- `run/build_fig7_article_comparison.sh`: atualiza o caso-base da Fig. 7, gera uma versao sem titulo e monta a comparacao `artigo x reproducao`.
