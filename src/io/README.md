# Inventario de `src/io/`

Esta pasta implementa a camada de I/O do projeto: leitura do schema JSON interno, derivacao de caminhos de saida e montagem dos relatorios estruturados.

## Subpastas

Esta pasta nao possui subpastas.

## Arquivos

- `coupler_io.cpp`: implementa o parsing e os relatorios do solver `solve_coupler`.
- `fig10_io.cpp`: implementa o parsing e os relatorios JSON/CSV da reproducao da Fig. 10.
- `fig11_io.cpp`: implementa o parsing e os relatorios JSON/CSV da reproducao da Fig. 11.
- `fig6_io.cpp`: implementa o parsing e os relatorios JSON/CSV da reproducao da Fig. 6.
- `fig7_io.cpp`: implementa o parsing e os relatorios JSON/CSV da reproducao da Fig. 7, incluindo linhas e intersecoes.
- `fig8_io.cpp`: implementa o parsing e os relatorios JSON/CSV da reproducao da Fig. 8.
- `readme.md`: este inventario da pasta `src/io/`.
- `schema_json.cpp`: implementa o parser JSON minimo e controlado pelo schema interno do repositorio.
- `single_guide_io.cpp`: implementa o parsing e os relatorios do solver de guia unico.
- `table1_io.cpp`: implementa o parsing e os relatorios JSON/CSV da reproducao da Tabela I.
- `text_io.cpp`: implementa utilitarios de texto, escape para JSON/CSV e leitura/escrita de arquivos.
