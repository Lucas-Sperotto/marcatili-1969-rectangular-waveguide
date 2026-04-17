# 10. Fluxo geral do repositório

Este arquivo explica o caminho completo entre um caso de entrada e uma figura reproduzida.

O objetivo é que um leitor novo consiga responder rapidamente a quatro perguntas:

1. onde entram os parâmetros do artigo;
2. onde a matemática é calculada;
3. onde os resultados são serializados;
4. onde os gráficos finais são gerados.

## Visão de alto nível

O repositório está organizado em quatro camadas principais:

- `data/input/`: casos de entrada em `JSON`;
- `src/apps/`: executáveis finos, um por tarefa científica;
- `src/io/` e `include/marcatili/io/`: leitura de `JSON` e escrita de `JSON`/`CSV`;
- `src/physics/` e `src/math/`: implementação do modelo de Marcatili e das rotinas numéricas de apoio.

Além disso:

- `scripts/run/*.sh` orquestram build, limpeza e reprodução em lote;
- `scripts/plot_fig*.py` transformam `CSV` em figuras `PNG`;
- `docs/` guarda a ponte artigo $\to$ implementação.

## Fluxo de Execução Passo a Passo

Para entender como o repositório funciona na prática, é útil seguir o fluxo de dados e execução desde a entrada do usuário até a geração do gráfico final. O processo pode ser dividido em três grandes etapas: **Simulação (C++)**, **Visualização (Python)** e **Orquestração (Shell)**.

### Etapa 1: Simulação em C++

1. **Entrada do Usuário (`data/input/`)**
    - Tudo começa com um arquivo de configuração `.json`, por exemplo, `data/input/reproduce_fig6.json`.
    - Este arquivo define todos os parâmetros da simulação: qual figura reproduzir, quais modelos usar (`closed_form`, `exact`), índices de refração (`n1`, `n2`, ...), geometria (`a_over_b`), faixa de varredura (`b_over_A4_min`, `b_over_A4_max`) e quais modos (`E_y:1:1`, etc.) calcular.

2. **Executável Principal (`src/apps/`)**
    - O usuário executa o aplicativo C++ correspondente, por exemplo, `./build/bin/reproduce_fig6`.
    - O `main()` em `src/apps/reproduce_fig6.cpp` é o ponto de entrada.

3. **Leitura e Parsing (`src/io/`)**
    - O aplicativo usa `ReadTextFile(...)` para ler o arquivo `.json` e depois chama o parser correspondente, por exemplo `ParseFigure6Config(...)`.
    - Essas funções validam a estrutura do JSON e o convertem em `structs` C++ tipadas, que são mais seguras e fáceis de usar no restante do código.

4. **Cálculo Físico (`src/physics/`)**
    - O aplicativo então chama a função principal da camada de física, como `SolveFigure6(...)`.
    - Esta função contém a lógica central:
        - Ela itera sobre a faixa de varredura definida no caso de entrada.
        - Para cada ponto, ela chama os solvers de guia único (`solve_single_guide`) para cada modo e modelo especificado.
        - Os solvers (`exact` ou `closed_form`) calculam `kx`, `ky`, e finalmente `kz`.
        - Os resultados são normalizados conforme a ordenada da Fig. 6 e armazenados em uma estrutura de dados de resultados.

5. **Escrita dos Resultados (`src/io/`)**
    - Após o término dos cálculos, o aplicativo chama novamente a camada de I/O para serializar os dados, por exemplo com `BuildFigure6JsonReport(...)` e `BuildFigure6CsvReport(...)`.
    - Os resultados são serializados em dois formatos em `data/output/`:
        - Um arquivo `.csv` contendo os dados numéricos prontos para plotagem.
        - Um arquivo `.json` de resumo, contendo os metadados da simulação e os resultados.

### Etapa 2: Visualização em Python

6. **Script de Plotagem (`scripts/`)**
    - Com os dados numéricos salvos, um script Python como `scripts/plot_fig6.py` é executado.
    - Ele usa bibliotecas como `pandas` para ler o `.csv` e `matplotlib` para gerar o gráfico, replicando o estilo do artigo original.
    - O gráfico final é salvo como um arquivo `.png` em `data/output/`.

### Etapa 3: Orquestração com Shell

7. **Scripts de Automação (`scripts/run/`)**
    - Para evitar a execução manual de cada passo, scripts de shell como `clean_build_reproduce_all.sh` ou `reproduce_fig6_panels.sh` automatizam todo o processo.
    - Esses scripts invocam o `cmake` para compilar o projeto, executam os binários C++ com os arquivos de entrada corretos e, em seguida, executam os scripts Python para gerar todos os gráficos de uma só vez. Isso garante a reprodutibilidade completa com um único comando.

## Fluxograma

Arquivo Mermaid equivalente:
[docs/diagrams/fluxo_geral.mmd](diagrams/fluxo_geral.mmd)

```mermaid
graph TD
    subgraph User
        A[Configuração JSON] --> B{Execução de Script};
    end

    subgraph "Orquestração (scripts/run/)"
        B -- 1. Compila --> C{CMake};
        B -- 2. Executa --> D[Apps C++];
        B -- 4. Executa --> G[Scripts Python];
    end

    subgraph "Código Fonte (src/, include/)"
        D -- Chama --> E[Camada de Física];
        E -- Usa --> F[Camada de Matemática];
    end

    subgraph "Dados (data/)"
        A --> D;
        D -- 3. Escreve --> H[Resultados CSV/JSON];
        H --> G;
    end

    subgraph "Artefatos Finais"
        G -- 5. Gera --> I[Gráficos PNG];
    end

    style User fill:#cde4ff
    style "Orquestração (scripts/run/)" fill:#e1d5e7
    style "Código Fonte (src/, include/)" fill:#d5e8d4
    style "Dados (data/)" fill:#f8cecc
    style "Artefatos Finais" fill:#fff2cc
```

## Como um `JSON` vira um cálculo

Cada executável segue a mesma ideia:

1. ler um arquivo de entrada;
2. chamar um parser específico da camada `io`;
3. obter uma configuração tipada;
4. chamar uma função `Solve*` na camada física;
5. serializar o resultado.

Exemplos:

- `solve_single_guide` usa `ParseSingleGuideConfig(...)` e depois `SolveSingleGuide(...)`;
- `reproduce_fig6` usa `ParseFigure6Config(...)` e depois `SolveFigure6(...)`;
- `solve_coupler` usa `ParseCouplerPointConfig(...)` e depois `SolveCouplerPoint(...)`.

Isso é importante pedagogicamente: o executável não deve esconder a física. Ele apenas encadeia:

$$
\text{JSON} \rightarrow \text{configuração tipada} \rightarrow \text{solver} \rightarrow \text{JSON/CSV}.
$$

## Papel da camada `io`

A camada `io` não é onde a física deve viver. A função dela é:

- interpretar o schema de entrada;
- validar campos básicos;
- converter texto em tipos C++;
- escrever relatórios legíveis por humanos e por scripts.

Hoje o parser JSON do repositório é **deliberadamente restrito** ao schema atual do projeto. Isso significa:

- ele funciona bem para os arquivos de caso existentes;
- ele não deve ser tratado como um parser JSON geral;
- para novos casos, o padrão preferido é usar objetos JSON explícitos, e não strings compactas.

Essa limitação é intencional nesta fase para manter o núcleo pequeno, auditável e sem dependências pesadas.

## Papel da camada `physics`

A camada `physics` concentra o significado científico do repositório.

Ela contém, por exemplo:

- `single_guide.cpp`: guia retangular único;
- `slab_guide.cpp`: limite de lâmina;
- `metal_guide.cpp`: caso da Fig. 8 com fronteira metalizada / baixa impedância;
- `coupler.cpp`: acoplamento normalizado da Seção IV;
- `fig6.cpp`, `fig7.cpp`, `fig8.cpp`, `fig10.cpp`, `fig11.cpp`, `table1.cpp`: organizadores de sweep e reprodução.

Um ponto importante: os arquivos `fig*.cpp` não reinventam a física. Eles pegam um solver central e o executam muitas vezes ao longo de uma varredura de parâmetros.

## Papel da camada `math`

A camada `math` é pequena de propósito. Ela contém utilidades reutilizáveis:

- `root_finding.cpp`: solução numérica de raízes por bisseção;
- `waveguide_math.cpp`: escalas como $A_v$, profundidades de penetração e limites seguros do domínio numérico.

Essa separação ajuda o leitor a distinguir:

- física do artigo;
- álgebra de normalização;
- decisão numérica de implementação.

## Como os executáveis se encaixam

Os executáveis em `src/apps/` são pontos de entrada reprodutíveis.

Eles são intencionalmente finos. Em geral, fazem apenas isto:

1. ler entrada com `ReadTextFile(...)`;
2. fazer parse com `Parse*Config(...)`;
3. resolver com `Solve*(...)`;
4. gerar saída com `Build*JsonReport(...)` e `Build*CsvReport(...)`.

Essa opção de arquitetura facilita:

- testes;
- revisão científica;
- rastreabilidade artigo $\to$ código.

## Como os scripts `run/*.sh` se encaixam

Os scripts em `scripts/run/` automatizam a rotina do projeto.

Os principais papéis são:

- `build.sh`: configurar e compilar;
- `clean.sh`: limpar artefatos;
- `reproduce_all.sh`: rodar todos os casos principais;
- `clean_build_reproduce_all.sh`: pipeline único de limpeza, build e reprodução;
- `build_fig*_article_comparison.sh`: montar comparações artigo $\times$ reprodução.

Ou seja, o shell coordena a execução, mas não substitui o núcleo científico.

## Como os scripts Python se encaixam

Os scripts `plot_fig*.py` e `compare_fig*.py` ficam fora do C++ por uma razão importante:

- o C++ produz dados;
- o Python produz visualização.

Esse desacoplamento ajuda a manter o projeto reprodutível e auditável:

- se um gráfico estiver ruim, podemos revisar o script sem tocar na matemática;
- se a matemática mudar, podemos regenerar as figuras a partir dos novos `CSV`.

## Exemplo completo

Um exemplo típico para a Fig. 6 é:

1. `data/input/fig6/SG-006h.json` define geometria, índices, modos e solver;
2. `reproduce_fig6` lê esse arquivo;
3. `src/io/fig6_io.cpp` converte o `JSON` para `Figure6Config`;
4. `src/physics/fig6.cpp` faz o sweep em $b/A_4$;
5. `src/physics/single_guide.cpp` ou `src/physics/slab_guide.cpp` resolve cada ponto;
6. `src/io/fig6_io.cpp` escreve `JSON` e `CSV`;
7. `scripts/plot_fig6.py` transforma o `CSV` em `PNG`.

## Leitura recomendada

Para entender o repositório de forma progressiva, a ordem sugerida é:

1. [README.md](../README.md)
2. [02_symbol_dictionary.md](02_symbol_dictionary.md)
3. [11_closed_form_vs_exact.md](11_closed_form_vs_exact.md)
4. [12_trilha_equacoes_para_codigo.md](12_trilha_equacoes_para_codigo.md)
5. [14_diagramas_de_fluxo_e_sequencia.md](14_diagramas_de_fluxo_e_sequencia.md)
6. [15_roteiro_de_estudo.md](15_roteiro_de_estudo.md)
