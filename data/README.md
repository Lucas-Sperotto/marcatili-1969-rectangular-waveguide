# data/

Esta pasta concentra os arquivos de entrada e os artefatos de saída do fluxo reprodutível do projeto.

## Subpastas

- `input/`: arquivos JSON de configuração aceitos pelos executáveis C++.
- `input/fig6/`: casos separados para cada painel da Fig. 6.
- `output/`: artefatos gerados automaticamente — relatórios JSON, tabelas CSV e imagens PNG.
- `output/fig6/`: painéis individuais da Fig. 6 (CSV, JSON e PNG por painel).

## Notas

Os arquivos em `output/` são gerados pelo fluxo de reprodução e não devem ser editados manualmente.
Para regenerar todos os artefatos: `./scripts/run.sh reproduce`.
