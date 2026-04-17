# 14. Diagramas de Fluxo e Sequência

Estes diagramas oferecem uma visão geral da arquitetura do repositório e do fluxo de execução para as principais aplicações. Eles são uma excelente ferramenta para entender como as diferentes partes do código colaboram.

Os arquivos-fonte canônicos `.mmd` preservados em `docs/diagrams/` são:

- `fluxo_geral.mmd`
- `sequence_solve_single_guide.mmd`
- `sequence_reproduce_fig6.mmd`
- `sequence_reproduce_fig7.mmd`
- `sequence_coupler_fig10_fig11.mmd`

## 1. Arquitetura Geral

Arquivo Mermaid equivalente:
[docs/diagrams/fluxo_geral.mmd](diagrams/fluxo_geral.mmd)

Este diagrama mostra como as diferentes partes do projeto se interconectam.

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

## 2. `solve_single_guide`

Arquivo Mermaid equivalente:
[docs/diagrams/sequence_solve_single_guide.mmd](diagrams/sequence_solve_single_guide.mmd)

```mermaid
sequenceDiagram
    participant U as Usuário
    participant S as Shell
    participant A as app solve_single_guide
    participant IO as camada io
    participant P as camada physics
    participant FS as arquivos JSON/CSV

    U->>S: ./build/bin/solve_single_guide input.json output.json
    S->>A: argumentos da linha de comando
    A->>IO: ReadTextFile(input.json)
    IO-->>A: texto JSON
    A->>IO: ParseSingleGuideConfig(...)
    IO-->>A: SingleGuideConfig
    A->>P: SolveSingleGuide(config)
    P-->>A: SingleGuideResult
    A->>IO: BuildSingleGuideJsonReport(...)
    A->>IO: BuildSingleGuideCsvReport(...)
    IO->>FS: escreve output.json e output.csv
    A-->>S: mensagem de sucesso
```

## 3. `reproduce_fig6`

Arquivo Mermaid equivalente:
[docs/diagrams/sequence_reproduce_fig6.mmd](diagrams/sequence_reproduce_fig6.mmd)

```mermaid
sequenceDiagram
    participant U as Usuário
    participant S as Shell
    participant A as app reproduce_fig6
    participant IO as camada io
    participant P as physics fig6
    participant G as physics single/slab
    participant FS as arquivos JSON/CSV
    participant PY as scripts/plot_fig6.py

    U->>S: ./build/bin/reproduce_fig6 input.json output.json
    S->>A: argumentos
    A->>IO: ParseFigure6Config(...)
    IO-->>A: Figure6Config
    A->>P: SolveFigure6(config)
    loop para cada variante, solver, modo e ponto do sweep
        P->>G: SolveSingleGuide(...) ou SolveSlabGuide(...)
        G-->>P: resultado pontual
    end
    P-->>A: Figure6Result
    A->>IO: BuildFigure6JsonReport(...)
    A->>IO: BuildFigure6CsvReport(...)
    IO->>FS: escreve output.json e output.csv
    U->>S: ./scripts/plot_fig6.py output.csv -o output.png
    S->>PY: csv + opções
    PY->>FS: lê CSV e escreve PNG
```

## 4. `reproduce_fig7`

Arquivo Mermaid equivalente:
[docs/diagrams/sequence_reproduce_fig7.mmd](diagrams/sequence_reproduce_fig7.mmd)

```mermaid
sequenceDiagram
    participant U as Usuário
    participant S as Shell
    participant A as app reproduce_fig7
    participant IO as camada io
    participant P as physics fig7
    participant FS as arquivos JSON/CSV
    participant PY as scripts/plot_fig7.py

    U->>S: ./build/bin/reproduce_fig7 input.json output.json
    S->>A: argumentos
    A->>IO: ParseFigure7Config(...)
    IO-->>A: Figure7Config
    A->>P: SolveFigure7(config)
    P-->>A: Figure7Result
    A->>IO: BuildFigure7JsonReport(...)
    A->>IO: BuildFigure7LinesCsvReport(...)
    A->>IO: BuildFigure7IntersectionsCsvReport(...)
    IO->>FS: escreve JSON + lines.csv + intersections.csv
    U->>S: ./scripts/plot_fig7.py lines.csv --intersections-csv intersections.csv -o output.png
    S->>PY: dois CSVs + opções
    PY->>FS: lê CSVs e escreve PNG
```

## 5. `solve_coupler`, `reproduce_fig10` e `reproduce_fig11`

Arquivo Mermaid equivalente:
[docs/diagrams/sequence_coupler_fig10_fig11.mmd](diagrams/sequence_coupler_fig10_fig11.mmd)

```mermaid
sequenceDiagram
    participant U as Usuário
    participant S as Shell
    participant A as apps do acoplador
    participant IO as camada io
    participant F as physics fig10/fig11
    participant C as physics coupler
    participant FS as arquivos JSON/CSV
    participant PY as scripts Python de plot

    alt solve_coupler
        U->>S: ./build/bin/solve_coupler input.json output.json
        S->>A: argumentos
        A->>IO: ParseCouplerPointConfig(...)
        IO-->>A: CouplerPointConfig
        A->>C: SolveCouplerPoint(config)
        C-->>A: CouplerPointResult
        A->>IO: BuildCouplerPointJsonReport(...)
        A->>IO: BuildCouplerPointCsvReport(...)
        IO->>FS: escreve JSON + CSV
    else reproduce_fig10
        U->>S: ./build/bin/reproduce_fig10 input.json output.json
        S->>A: argumentos
        A->>IO: ParseFigure10Config(...)
        IO-->>A: Figure10Config
        A->>F: SolveFigure10(config)
        loop para cada curva a/A5 e ponto c/a
            F->>C: SolveCouplerPoint(...)
            C-->>F: ponto normalizado de acoplamento
        end
        F-->>A: Figure10Result
        A->>IO: BuildFigure10JsonReport(...)
        A->>IO: BuildFigure10CsvReport(...)
        IO->>FS: escreve JSON + CSV
        U->>S: ./scripts/plot_fig10.py output.csv -o output.png
        S->>PY: csv + opções
        PY->>FS: lê CSV e escreve PNG
    else reproduce_fig11
        U->>S: ./build/bin/reproduce_fig11 input.json output.json
        S->>A: argumentos
        A->>IO: ParseFigure11Config(...)
        IO-->>A: Figure11Config
        A->>F: SolveFigure11(config)
        loop para cada n1/n5, a/A5 e ponto c/a
            F->>C: SolveCouplerPoint(...)
            C-->>F: ponto normalizado de acoplamento
        end
        F-->>A: Figure11Result
        A->>IO: BuildFigure11JsonReport(...)
        A->>IO: BuildFigure11CsvReport(...)
        IO->>FS: escreve JSON + CSV
        U->>S: ./scripts/plot_fig11.py output.csv -o output.png
        S->>PY: csv + opções
        PY->>FS: lê CSV e escreve PNG
    end
```

## Observação final

Os diagramas deixam explícita uma decisão de arquitetura do projeto:

- o C++ produz os artefatos científicos;
- o Python cuida da visualização;
- os scripts `run/*.sh` organizam a execução reproduzível.

Essa separação ajuda muito quando queremos revisar:

- a matemática sem mexer na apresentação;
- a apresentação sem mexer na matemática.


<!-- NAV START -->
---

**Navegação:** [Anterior](13_validacao_e_limites_do_modelo.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](15_roteiro_de_estudo.md)

<!-- NAV END -->
