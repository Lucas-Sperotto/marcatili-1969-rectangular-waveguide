# Inventario de `src/physics/`

Esta pasta implementa a camada fisica do repositorio: solvers pontuais, limites especiais e rotinas de sweep usadas para reproduzir figuras e tabela do artigo.

## Subpastas

Esta pasta nao possui subpastas.

## Arquivos

- `coupler.cpp`: implementa o solver de um ponto do acoplador e a avaliacao do modelo normalizado usado nas Figs. 10 e 11.
- `fig10.cpp`: implementa o sweep numerico e a montagem dos resultados da Fig. 10.
- `fig11.cpp`: implementa o sweep numerico e a montagem dos resultados da Fig. 11.
- `fig6.cpp`: implementa o sweep numerico de um painel da Fig. 6, incluindo casos retangulares e de lamina.
- `fig7.cpp`: implementa a construcao do nomograma da Fig. 7 e de suas intersecoes.
- `fig8.cpp`: implementa o sweep numerico da Fig. 8 a partir do solver de guia metalizado.
- `metal_guide.cpp`: implementa o solver do guia metalizado nas variantes `closed_form` e `exact`.
- `readme.md`: este inventario da pasta `src/physics/`.
- `single_guide.cpp`: implementa o solver principal de guia unico nas variantes `closed_form` e `exact`.
- `slab_guide.cpp`: implementa o limite de lamina, reutilizando a interface do guia unico.
- `table1.cpp`: implementa a busca numerica de cutoff e a montagem dos resultados da Tabela I.
