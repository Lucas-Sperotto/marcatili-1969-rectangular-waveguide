# marcatili-1969-rectangular-waveguide

Repositório em C++17 para reproduzir, de forma didática e reprodutível, o artigo *Dielectric Rectangular Waveguide and Directional Coupler for Integrated Optics* (Marcatili, 1969).

## Objetivo

Este projeto busca reconstruir:

1. a formulação do guia dielétrico retangular;
2. as aproximações analíticas usadas por Marcatili;
3. as figuras e tabelas centrais do artigo;
4. o modelo do acoplador direcional;
5. uma trilha clara entre documentação, implementação e validação.

Nesta etapa, a base do projeto já inclui:

1. um solver do guia único com versões aproximada e exata;
2. a reprodução operacional da Fig. 6, incluindo casos retangulares e limites de lâmina;
3. uma primeira reprodução do nomograma da Fig. 7;
4. placeholders organizados para as demais figuras e para o acoplador.

## Estrutura

- `docs/`: documentação, tradução, notas editoriais e material matemático.
- `docs/refs/`: referências, PDF do artigo, notas de OCR e verificações manuais.
- `data/`: arquivos de entrada e saídas numéricas em `CSV` e `JSON`.
- `include/`: cabeçalhos do núcleo C++.
- `src/`: implementação compartilhada.
- `src/apps/`: executáveis do projeto.
- `scripts/`: scripts Python externos e scripts de automação.
- `tests/`: testes automáticos e casos de verificação.
- `scripts/run/`: scripts de build, limpeza e reprodução.

## Estado atual dos executáveis

- `solve_single_guide`: resolve um caso pontual do guia único com solver `closed_form` ou `exact` e gera saídas em `JSON` e `CSV`.
- `solve_coupler`: placeholder.
- `reproduce_fig6`: executa um sweep em $b/A_4$ e gera curvas numéricas em `CSV` e resumo em `JSON` para um painel de Fig. 6, com suporte a solver aproximado, exato e ao limite de lâmina.
- `reproduce_fig7`: reproduz o nomograma da Fig. 7 como conjunto de retas modais, retas de $C$ e interseções de referência em `JSON` e `CSV`.
- `reproduce_fig8`: placeholder.
- `reproduce_fig10`: placeholder.
- `reproduce_fig11`: placeholder.
- `reproduce_table1`: placeholder.

Todos aceitam um arquivo de entrada:

```bash
./build/bin/solve_single_guide data/input/solve_single_guide.json
./build/bin/reproduce_fig6 data/input/reproduce_fig6.json data/output/reproduce_fig6.json
./scripts/plot_fig6.py data/output/reproduce_fig6.csv -o data/output/reproduce_fig6.png
./build/bin/reproduce_fig7 data/input/reproduce_fig7.json data/output/reproduce_fig7.json
./scripts/plot_fig7.py data/output/reproduce_fig7.lines.csv --intersections-csv data/output/reproduce_fig7.intersections.csv -o data/output/reproduce_fig7.png
./scripts/run/build_fig7_article_comparison.sh
```

## Build e execução

```bash
./scripts/run/build.sh
./scripts/run/reproduce_all.sh
./scripts/run/reproduce_fig6_panels.sh
./scripts/run/reproduce_fig7_nomogram.sh
./scripts/run/clean.sh
```

Se quiser executar os smoke tests registrados no `CTest`, rode:

```bash
RUN_TESTS=1 ./scripts/run/build.sh
```

## Convenções do projeto

- núcleo numérico em `C++17`;
- sem dependências desnecessárias nesta fase inicial;
- gráficos gerados apenas por scripts Python externos;
- cada executável deve receber um arquivo de entrada;
- saídas numéricas devem permanecer em `CSV` ou `JSON`;
- ambiguidades de OCR devem ser marcadas com `TODO`.
- o primeiro solver implementado deve sempre explicitar quais equações do artigo está usando.
- gráficos intermediários e finais devem nascer apenas dos artefatos numéricos em `CSV` e `JSON`.

## Roteiro de evolução

1. consolidar a documentação técnica e o dicionário de símbolos em `docs/`;
2. refinar a transcrição e a validação cruzada da Fig. 6;
3. transformar a primeira base da Fig. 7 em comparação quantitativa com o artigo;
4. reproduzir Fig. 8 e Tabela I;
5. implementar o acoplador direcional e reproduzir Fig. 10 e Fig. 11.
