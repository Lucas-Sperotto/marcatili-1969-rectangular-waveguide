# marcatili-1969-rectangular-waveguide

Repositório em C++17 para reproduzir, de forma didática e reprodutível, o artigo *Dielectric Rectangular Waveguide and Directional Coupler for Integrated Optics* (Marcatili, 1969).

## Objetivo

Este projeto busca reconstruir:

1. a formulação do guia dielétrico retangular;
2. as aproximações analíticas usadas por Marcatili;
3. as figuras e tabelas centrais do artigo;
4. o modelo do acoplador direcional;
5. uma trilha clara entre documentação, implementação e validação.

Nesta etapa, o repositório foi preparado como uma base limpa para evolução. A matemática completa ainda não foi implementada; os executáveis atuais são placeholders que recebem um arquivo de entrada e geram um relatório `JSON`.

## Estrutura inicial

- `docs/`: documentação, tradução, notas editoriais e material matemático.
- `refs/`: referências, PDF do artigo, notas de OCR e verificações manuais.
- `data/`: arquivos de entrada e saídas numéricas em `CSV` e `JSON`.
- `include/`: cabeçalhos do núcleo C++.
- `src/`: implementação compartilhada.
- `apps/`: executáveis do projeto.
- `scripts/`: scripts Python externos para gráficos e apoio à reprodução.
- `tests/`: testes automáticos e casos de verificação.
- `run/`: scripts de build, limpeza e reprodução.

## Executáveis placeholder

- `solve_single_guide`
- `solve_coupler`
- `reproduce_fig6`
- `reproduce_fig7`
- `reproduce_fig8`
- `reproduce_fig10`
- `reproduce_fig11`
- `reproduce_table1`

Todos aceitam um arquivo de entrada:

```bash
./build/bin/solve_single_guide data/input/solve_single_guide.json
./build/bin/reproduce_fig6 data/input/reproduce_fig6.json data/output/reproduce_fig6.json
```

## Build e execução

```bash
./run/build.sh
./run/reproduce_all.sh
./run/clean.sh
```

Se quiser executar os smoke tests registrados no `CTest`, rode:

```bash
RUN_TESTS=1 ./run/build.sh
```

## Convenções do projeto

- núcleo numérico em `C++17`;
- sem dependências desnecessárias nesta fase inicial;
- gráficos gerados apenas por scripts Python externos;
- cada executável deve receber um arquivo de entrada;
- saídas numéricas devem permanecer em `CSV` ou `JSON`;
- ambiguidades de OCR devem ser marcadas com `TODO`.

## Roteiro de evolução

1. consolidar a documentação técnica e o dicionário de símbolos em `docs/`;
2. implementar o solver do guia único e validar as equações principais;
3. reproduzir Fig. 6, Fig. 7, Fig. 8 e Tabela I;
4. implementar o acoplador direcional e reproduzir Fig. 10 e Fig. 11;
5. ampliar a suíte de testes e a comparação com os resultados do artigo.
