# Inventario de `include/marcatili/io/`

Esta pasta declara a camada de I/O do projeto: leitura de JSONs controlados pelo schema do repositorio, montagem de relatorios e utilitarios de texto.

## Subpastas

Esta pasta nao possui subpastas.

## Arquivos

- `coupler_io.hpp`: declaracoes para ler a entrada do acoplador e montar os relatorios JSON/CSV correspondentes.
- `fig10_io.hpp`: declaracoes de parsing e serializacao da reproducao da Fig. 10.
- `fig11_io.hpp`: declaracoes de parsing e serializacao da reproducao da Fig. 11.
- `fig6_io.hpp`: declaracoes de parsing e serializacao da reproducao da Fig. 6.
- `fig7_io.hpp`: declaracoes de parsing e serializacao da reproducao da Fig. 7, incluindo o CSV de intersecoes.
- `fig8_io.hpp`: declaracoes de parsing e serializacao da reproducao da Fig. 8.
- `readme.md`: este inventario da pasta `include/marcatili/io/`.
- `schema_json.hpp`: parser JSON minimo e controlado, restrito ao subconjunto do schema usado pelo repositorio.
- `single_guide_io.hpp`: declaracoes para ler a entrada do guia unico e produzir seus relatorios de saida.
- `table1_io.hpp`: declaracoes de parsing e serializacao da reproducao da Tabela I.
- `text_io.hpp`: utilitarios de leitura/escrita de texto, escape para JSON/CSV e troca de extensao de caminho.
