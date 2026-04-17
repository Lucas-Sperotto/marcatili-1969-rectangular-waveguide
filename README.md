# marcatili-1969-rectangular-waveguide

Repositório em **C++17** para reproduzir, de forma didática e reprodutível, o artigo **“Dielectric Rectangular Waveguide and Directional Coupler for Integrated Optics”** (Marcatili, 1969).

O foco do projeto é ligar, de maneira clara, três camadas:

- **artigo original**;
- **documentação técnica e pedagógica**;
- **implementação numérica reprodutível**.

## Visão geral

O repositório já inclui:

- solver do **guia único** com versões `closed_form` e `exact`;
- reprodução operacional da **Fig. 6**;
- primeira reprodução do nomograma da **Fig. 7**;
- reprodução operacional da **Fig. 8** com comparação ao scan do artigo;
- reprodução operacional da **Fig. 10**;
- reprodução operacional da **Fig. 11**;
- base organizada para expansão do **acoplador direcional** e dos **casos perturbados**.

## Leitura rápida

Uma sequência recomendada para entender o projeto é:

1. ler este `README.md`;
2. abrir os arquivos de entrada em `data/input/`;
3. ler os executáveis em `src/apps/`;
4. descer para `src/io/`, `src/math/` e `src/physics/`;
5. por fim, ver os scripts de plot em `scripts/`.

Documentos recomendados para a trilha técnica:

- [Fluxo geral do repositório](docs/10_fluxo_geral_do_repositorio.md)
- [Closed form vs exact](docs/11_closed_form_vs_exact.md)
- [Trilha equações → código](docs/12_trilha_equacoes_para_codigo.md)
- [Validação e limites do modelo](docs/13_validacao_e_limites_do_modelo.md)
- [Diagramas de fluxo e sequência](docs/14_diagramas_de_fluxo_e_sequencia.md)
- [Roteiro de estudo](docs/15_roteiro_de_estudo.md)

## Tradução do artigo

A tradução e organização comentada do artigo estão distribuídas nos arquivos abaixo:

- [Resumo inicial](docs/00_resumo.md)
- [1. Introduction](docs/01_introduction.md)
- [2. Formulação do problema de valor de contorno](docs/02_formulacao_do_problema_de_valor_de_contorno.md)
- [Dicionário de símbolos](docs/02_symbol_dictionary.md)
- [3.1 Modos \(E^y\)](docs/03.1_modos_Ey.md)
- [3.2 Modos \(E^x\)](docs/03.2_modos_Ex.md)
- [3.3 Exemplos](docs/03.3_exemplos.md)
- [3. Guia imerso em vários dielétricos](docs/03_guia_imerso_em_varios_dieletricos.md)
- [4. Acoplador direcional](docs/04_acoplador_direcional.md)
- [5. Acoplador direcional com guias ligeiramente diferentes](docs/05_Acoplador%20direcional%20constru%C3%ADdo%20com%20guias%20ligeiramente%20diferentes.md)
- [6. Resumo e conclusões](docs/06_resumo_e_conclusoes.md)
- [7. Apêndice A](docs/07_apendice_A.md)
- [8. Referências](docs/08_referencias.md)

Referências de apoio:

- [PDF do artigo](docs/refs/j.1538-7305.1969.tb01166.x.pdf)
- [Notas sobre figuras](docs/00.1_figuras.md)
- [Casos de teste](docs/00.2_casos_de_teste.md)
- [Relatório de divergências das figuras](docs/00.4_relatorio_divergencias_figuras.md)

## Estrutura do repositório

- `docs/`: documentação técnica, tradução, notas editoriais e material matemático;
- `docs/refs/`: referências e PDF do artigo;
- `data/input/`: arquivos de entrada dos casos;
- `data/output/`: saídas numéricas e figuras geradas;
- `include/`: cabeçalhos do núcleo C++;
- `src/`: implementação compartilhada;
- `src/apps/`: executáveis;
- `scripts/`: scripts Python e automações;
- `scripts/run/`: build, limpeza e reprodução;
- `tests/`: testes e verificações.

## Estado atual dos executáveis

- `solve_single_guide`: resolve um caso do guia único com solver `closed_form` ou `exact`;
- `solve_coupler`: resolve um ponto do acoplador no modelo normalizado da Eq. (34);
- `reproduce_fig6`: reproduz curvas da Fig. 6;
- `reproduce_fig7`: reproduz o nomograma da Fig. 7;
- `reproduce_fig8`: reproduz o caso metalizado da Fig. 8;
- `reproduce_fig10`: reproduz a curva de acoplamento da Fig. 10;
- `reproduce_fig11`: reproduz a curva de acoplamento da Fig. 11;
- `reproduce_table1`: compara as dimensões monomodo da Tabela I com os cálculos do repositório.

## Execução rápida

Todos os executáveis recebem um arquivo de entrada.

```bash
./build/bin/solve_single_guide data/input/solve_single_guide.json
./build/bin/solve_coupler data/input/solve_coupler.json data/output/solve_coupler.json

./build/bin/reproduce_fig6 data/input/reproduce_fig6.json data/output/reproduce_fig6.json
./scripts/plot_fig6.py data/output/reproduce_fig6.csv -o data/output/reproduce_fig6.png

./build/bin/reproduce_fig7 data/input/reproduce_fig7.json data/output/reproduce_fig7.json
./scripts/plot_fig7.py data/output/reproduce_fig7.lines.csv --intersections-csv data/output/reproduce_fig7.intersections.csv -o data/output/reproduce_fig7.png

./build/bin/reproduce_fig8 data/input/reproduce_fig8.json data/output/reproduce_fig8.json
./scripts/plot_fig8.py data/output/reproduce_fig8.csv -o data/output/reproduce_fig8.png

./build/bin/reproduce_fig10 data/input/reproduce_fig10.json data/output/reproduce_fig10.json
./scripts/plot_fig10.py data/output/reproduce_fig10.csv -o data/output/reproduce_fig10.png

./build/bin/reproduce_fig11 data/input/reproduce_fig11.json data/output/reproduce_fig11.json
./scripts/plot_fig11.py data/output/reproduce_fig11.csv -o data/output/reproduce_fig11.png
````

## Build e automação

```bash
./scripts/run/build.sh
./scripts/run/clean_build_reproduce_all.sh
./scripts/run/reproduce_all.sh
./scripts/run/reproduce_fig6_panels.sh
./scripts/run/reproduce_fig7_nomogram.sh
./scripts/run/reproduce_fig8_case.sh
./scripts/run/reproduce_fig10_case.sh
./scripts/run/reproduce_fig11_case.sh
./scripts/run/check_reproduction.sh
./scripts/run/clean.sh
```

Para remover também saídas rastreadas pelo Git:

```bash
CLEAN_TRACKED_OUTPUT=1 ./scripts/run/clean.sh
```

Para executar os smoke tests registrados no `CTest`:

```bash
RUN_TESTS=1 ./scripts/run/build.sh
```

## `closed_form` e `exact`

Esses dois termos aparecem em vários pontos do repositório:

* `closed_form`: usa as fórmulas aproximadas em forma fechada derivadas por Marcatili;
* `exact`: resolve numericamente as equações transcendentais do modelo de Marcatili adotado no projeto.

Importante: aqui `exact` **não** significa resolver o problema vetorial 2D completo sem aproximações. Significa resolver numericamente, de forma exata, a versão transcendental do modelo implementado no repositório.

Veja também: [docs/11_closed_form_vs_exact.md](docs/11_closed_form_vs_exact.md)

## Convenções do projeto

* núcleo numérico em `C++17`;
* dependências externas mínimas nesta fase;
* gráficos gerados apenas por scripts Python externos;
* cada executável deve receber um arquivo de entrada;
* o parser JSON interno é restrito ao schema do projeto;
* novos casos devem preferir arrays de objetos JSON;
* saídas numéricas devem permanecer em `CSV` e `JSON`;
* ambiguidades de OCR devem ser marcadas com `TODO`;
* cada solver novo deve explicitar quais equações do artigo está usando;
* gráficos finais devem nascer apenas dos artefatos numéricos produzidos pelo código.

## Mapa artigo → código

Uma leitura prática do projeto pode seguir esta correspondência:

* **Seções 2 e 3 do artigo** → `solve_single_guide`, `reproduce_fig6`, `reproduce_fig7`, `reproduce_fig8`
* **Seção 4 do artigo** → `solve_coupler`, `reproduce_fig10`, `reproduce_fig11`
* **Tabela I** → `reproduce_table1`
* **Apêndice A** → base matemática do acoplador e das equações transcendentais

## Estado de reprodução

* **Fig. 6**: reprodução operacional implementada;
* **Fig. 7**: reprodução operacional implementada, ainda em consolidação quantitativa;
* **Fig. 8**: reprodução operacional implementada com comparação ao artigo;
* **Fig. 10**: reprodução operacional implementada com comparação ao artigo;
* **Fig. 11**: reprodução operacional implementada com comparação ao artigo;
* **Tabela I**: comparação operacional implementada;
* **casos perturbados da Seção V**: ainda não implementados.

## Roteiro de evolução

1. consolidar a documentação técnica e o dicionário de símbolos;
2. refinar a validação cruzada da Fig. 6;
3. consolidar a comparação quantitativa da Fig. 7;
4. consolidar as comparações artigo × reprodução das Figs. 8, 10 e 11;
5. expandir o núcleo do acoplador para os exemplos numéricos da Seção IV;
6. ligar esse núcleo ao `solve_coupler`;
7. avançar para os casos perturbados da Seção V.
