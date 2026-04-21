# Guia de desenvolvimento

Este documento reúne as convenções do repositório e o roteiro de evolução planejado. Destina-se a quem quer contribuir com código, scripts ou documentação.

## Convenções do projeto

### Núcleo numérico

- Implementado em **C++17** com dependências externas mínimas.
- Cada solver deve declarar explicitamente quais equações do artigo está usando.
- Novos solvers devem cobrir tanto o caminho `closed_form` quanto o `exact`.

### Entradas e saídas

- Cada executável recebe um arquivo JSON de entrada; o schema é controlado pelo parser interno.
- Novos casos devem usar **arrays de objetos JSON** (não strings compactas legadas).
- Saídas numéricas devem permanecer em **CSV** e **JSON**; gráficos só em PNG gerados por scripts Python.
- Gráficos finais devem nascer exclusivamente dos artefatos numéricos produzidos pelo código C++.

### Documentação e rastreabilidade

- Ambiguidades de OCR no artigo original devem ser marcadas com `TODO OCR`.
- Divergências entre reprodução e scan devem ser classificadas: científica vs. editorial.
- O solver `exact` **não** resolve o problema vetorial 2D completo — resolve a equação transcendental do modelo separável de Marcatili. Essa distinção deve ser reforçada em toda a documentação nova.

### Testes

- Regressões quantitativas devem ser adicionadas ao `tests/regression_checks.cpp` para qualquer ponto numérico consolidado.
- Tolerâncias devem ser documentadas junto ao teste.

## Mapa artigo → código

| Seção do artigo | Executável(is) |
| --- | --- |
| Seções 2 e 3 — guia único | `solve_single_guide`, `reproduce_fig6`, `reproduce_fig7`, `reproduce_fig8` |
| Seção 4 — acoplador direcional | `solve_coupler`, `reproduce_fig10`, `reproduce_fig11` |
| Tabela I — cutoff monomodo | `reproduce_table1` |
| Apêndice A — base matemática | núcleo de `src/math/` e `src/physics/coupler.cpp` |
| Seção 5 — guias perturbados | *ainda não implementado* |

## Roteiro de evolução

A sequência recomendada de próximas contribuições, em ordem de prioridade:

1. **Fechar ambiguidades OCR centrais** — Fig. 8 (eixo e ramo intermediário) e Fig. 10 (família intermediária 1.0 vs. 1.6); ver [TODO.md](TODO.md).
2. **Incorporar referência de Jones/Goell** — necessária para validação independente do acoplador.
3. **Refinamento de fac-símile** — ajuste visual dos painéis `6c`, `6e` e `6j`; rótulos dos scripts `plot_fig*.py`.
4. **Expansão do acoplador** — cobrir o caso perturbado da Seção V com guias ligeiramente diferentes.
5. **Migração JSON** — converter casos canônicos remanescentes para arrays de objetos.

Para a lista completa de tarefas abertas, ver [TODO.md](TODO.md).

## O que não fazer cedo demais

- Refatorar fortemente a arquitetura C++ sem necessidade imediata.
- Trocar o parser JSON por uma solução mais pesada sem um caso de uso concreto.
- Perseguir fac-símile visual completo antes de fechar ambiguidades científicas.
- Apresentar o solver `exact` como solução vetorial rigorosa do problema 2D.
