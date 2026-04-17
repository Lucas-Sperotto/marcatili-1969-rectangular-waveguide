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
- `plot_fig8.py`: plota a reproducao atual da Fig. 8 a partir do `CSV` gerado por `reproduce_fig8`.
- `compare_fig8_article.py`: monta uma comparacao lado a lado entre o scan da Fig. 8 e a reproducao atual.
- `plot_fig10.py`: plota a reproducao atual da Fig. 10 a partir do `CSV` gerado por `reproduce_fig10`.
- `compare_fig10_article.py`: monta uma comparacao lado a lado entre o scan da Fig. 10 e a reproducao atual.
- `plot_fig11.py`: plota a reproducao atual da Fig. 11 a partir do `CSV` gerado por `reproduce_fig11`.
- `compare_fig11_article.py`: monta uma comparacao lado a lado entre o scan da Fig. 11 e a reproducao atual.
- `run/reproduce_fig6_panels.sh`: gera JSON, CSV e PNG para os paineis finitos de Fig. 6 atualmente configurados.
- `run/build_fig6_article_comparisons.sh`: atualiza os paineis numericos disponiveis e gera comparacoes `artigo x reproducao`, incluindo os limites de lamina `6d` e `6k`.
- `run/reproduce_fig7_nomogram.sh`: gera os artefatos numericos e o `PNG` do caso-base atualmente configurado para a Fig. 7.
- `run/build_fig7_article_comparison.sh`: atualiza o caso-base da Fig. 7, gera uma versao sem titulo e monta a comparacao `artigo x reproducao`.
- `run/reproduce_fig8_case.sh`: gera os artefatos numericos e o `PNG` do caso-base atualmente configurado para a Fig. 8.
- `run/build_fig8_article_comparison.sh`: atualiza o caso-base da Fig. 8, gera uma versao sem titulo e monta a comparacao `artigo x reproducao`.
- `run/reproduce_fig10_case.sh`: gera os artefatos numericos e o `PNG` do caso-base atualmente configurado para a Fig. 10.
- `run/build_fig10_article_comparison.sh`: atualiza o caso-base da Fig. 10, gera uma versao sem titulo e monta a comparacao `artigo x reproducao`.
- `run/reproduce_fig11_case.sh`: gera os artefatos numericos e o `PNG` do caso-base atualmente configurado para a Fig. 11.
- `run/build_fig11_article_comparison.sh`: atualiza o caso-base da Fig. 11, gera uma versao sem titulo e monta a comparacao `artigo x reproducao`.
