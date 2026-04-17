# Inventario de `data/input/`

Esta pasta guarda os arquivos JSON de entrada aceitos pelos executaveis do repositorio. Cada arquivo representa um caso reprodutivel que pode ser executado sem editar codigo.

## Subpastas

- `fig6/`: casos separados para os paineis da Fig. 6.

## Arquivos

- `readme.md`: este inventario da pasta `data/input/`.
- `reproduce_fig10.json`: caso-base para o executavel `reproduce_fig10`, responsavel pelo sweep da Fig. 10.
- `reproduce_fig11.json`: caso-base para o executavel `reproduce_fig11`, responsavel pelo sweep da Fig. 11.
- `reproduce_fig7.json`: caso-base para o executavel `reproduce_fig7`, que monta o nomograma da Fig. 7.
- `reproduce_fig8.json`: caso-base para o executavel `reproduce_fig8`, voltado ao guia metalizado da Fig. 8.
- `reproduce_table1.json`: configuracao da reproducao da Tabela I, incluindo busca de cutoffs e saidas CSV.
- `solve_coupler.json`: entrada de um ponto isolado do acoplador direcional no modelo normalizado.
- `solve_single_guide.json`: entrada principal do solver de guia unico no schema atual do repositorio.
- `solve_single_guide_legacy_flat.json`: variante de compatibilidade para testar o parser com um schema legado mais plano.
