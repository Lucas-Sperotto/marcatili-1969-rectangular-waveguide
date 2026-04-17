# Inventario de `include/marcatili/physics/`

Esta pasta declara as estruturas de configuracao, resultados e interfaces dos solvers fisicos do projeto.

## Subpastas

Esta pasta nao possui subpastas.

## Arquivos

- `coupler.hpp`: estruturas e funcoes do problema normalizado do acoplador direcional, base de `solve_coupler`, Fig. 10 e Fig. 11.
- `fig10.hpp`: configuracoes e resultados da reproducao da Fig. 10.
- `fig11.hpp`: configuracoes e resultados da reproducao da Fig. 11.
- `fig6.hpp`: configuracoes e resultados da reproducao da Fig. 6, incluindo os casos retangulares e de lamina.
- `fig7.hpp`: configuracoes e resultados da reproducao da Fig. 7, incluindo retas modais e retas de `C`.
- `fig8.hpp`: configuracoes e resultados da reproducao da Fig. 8 para o caso metalizado.
- `metal_guide.hpp`: interfaces do solver do guia metalizado em forma fechada e por raiz numerica.
- `readme.md`: este inventario da pasta `include/marcatili/physics/`.
- `single_guide.hpp`: estruturas principais do solver de guia unico, incluindo familia modal e modelo numerico.
- `slab_guide.hpp`: interfaces do limite de lamina, tratado como problema transversal 1D.
- `table1.hpp`: configuracoes e resultados da reproducao da Tabela I e da busca de cutoff monomodo.
