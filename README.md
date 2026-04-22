# marcatili-1969-rectangular-waveguide

[![DOI](https://zenodo.org/badge/1212433330.svg)](https://doi.org/10.5281/zenodo.19699346)

Reprodução didática e reprodutível do artigo clássico de fotônica integrada:

> **"Dielectric Rectangular Waveguide and Directional Coupler for Integrated Optics"**
> E. A. J. Marcatili — *Bell System Technical Journal*, vol. 48, pp. 2071–2102, [![DOI]]([https://doi.org/10.1002/j.1538-7305.1969.tb01166.x](https://doi.org/10.1002/j.1538-7305.1969.tb01166.x))

Implementado em **C++17**. O foco é conectar três camadas de forma explícita e rastreável:

```text
artigo original  ←→  documentação técnica  ←→  implementação numérica
```

---

## Figuras reproduzidas

| Figura | Descrição | Status |
| --- | --- | --- |
| Fig. 6 | Curvas de dispersão do guia único (11 painéis) | operacional |
| Fig. 7 | Nomograma de cutoff monomodo | operacional |
| Fig. 8 | Guia retangular com paredes metálicas | operacional |
| Fig. 10 | Acoplamento normalizado — família E^y | operacional |
| Fig. 11 | Acoplamento normalizado — família E^x | operacional |
| Tabela I | Dimensões de cutoff monomodo | operacional |

---

## Quick start

```bash
# 1. Compilar
./scripts/run.sh build

# 2. Reproduzir todas as figuras
./scripts/run.sh reproduce

# 3. Verificar artefatos obrigatórios
./scripts/run.sh check

# 4. Tudo de uma vez (clean + build + reproduce + check)
./scripts/run.sh full
```

Para compilar com testes:

```bash
RUN_TESTS=1 ./scripts/run.sh build
```

Para remover também saídas rastreadas pelo Git:

```bash
CLEAN_TRACKED_OUTPUT=1 ./scripts/run.sh clean
```

---

## Executáveis disponíveis

| Executável | Descrição |
| --- | --- |
| `solve_single_guide` | Resolve um caso isolado do guia único (`closed_form` ou `exact`) |
| `solve_coupler` | Resolve um ponto do acoplador no modelo normalizado da Eq. (34) |
| `reproduce_fig6` | Reproduz um painel da Fig. 6 |
| `reproduce_fig7` | Reproduz o nomograma da Fig. 7 |
| `reproduce_fig8` | Reproduz a Fig. 8 (guia metalizado) |
| `reproduce_fig10` | Reproduz a curva de acoplamento da Fig. 10 |
| `reproduce_fig11` | Reproduz a curva de acoplamento da Fig. 11 |
| `reproduce_table1` | Compara as dimensões de cutoff da Tabela I |

Cada executável recebe um arquivo JSON de entrada em `data/input/`.

---

## Estrutura do repositório

```text
├── data/
│   ├── input/          arquivos JSON de entrada dos executáveis
│   │   └── fig6/       casos por painel da Fig. 6
│   └── output/         saídas numéricas (CSV/JSON) e imagens (PNG)
│       └── fig6/       painéis individuais da Fig. 6
├── docs/               documentação técnica, tradução e material de apoio
│   └── refs/           PDF do artigo e referências
├── include/marcatili/  cabeçalhos públicos C++
│   ├── io/             interfaces de I/O
│   ├── math/           núcleo matemático
│   └── physics/        solvers físicos
├── scripts/            scripts Python de plot + run.sh
├── src/                implementações C++
│   ├── apps/           pontos de entrada dos executáveis
│   ├── io/             camada de parsing e serialização
│   ├── math/           funções matemáticas compartilhadas
│   └── physics/        solvers e rotinas de sweep
└── tests/              testes e regressões automáticas
```

---

## Documentação

### Trilha técnica recomendada

1. [Fluxo geral do repositório](docs/10_fluxo_geral_do_repositorio.md)
2. [Closed form vs exact](docs/11_closed_form_vs_exact.md)
3. [Trilha equações → código](docs/12_trilha_equacoes_para_codigo.md)
4. [Validação e limites do modelo](docs/13_validacao_e_limites_do_modelo.md)
5. [Diagramas de fluxo e sequência](docs/14_diagramas_de_fluxo_e_sequencia.md)
6. [Roteiro de estudo](docs/15_roteiro_de_estudo.md)

### Tradução comentada do artigo

- [Resumo](docs/00_resumo.md) — índice completo com todos os documentos
- [1. Introdução](docs/01_introduction.md)
- [2. Formulação do problema](docs/02_formulacao_do_problema_de_valor_de_contorno.md) · [Dicionário de símbolos](docs/02_symbol_dictionary.md)
- [3. Guia único — modos E^y](docs/03.1_modos_Ey.md) · [modos E^x](docs/03.2_modos_Ex.md) · [exemplos](docs/03.3_exemplos.md)
- [4. Acoplador direcional](docs/04_acoplador_direcional.md)
- [5. Guias ligeiramente diferentes](docs/05_Acoplador%20direcional%20constru%C3%ADdo%20com%20guias%20ligeiramente%20diferentes.md)
- [6. Resumo e conclusões](docs/06_resumo_e_conclusoes.md)
- [Apêndice A](docs/07_apendice_A.md) · [Referências](docs/08_referencias.md)

### Auditoria e validação

- [Auditoria técnica do modelo](docs/20_auditoria_tecnica_do_modelo.md)
- [Validação figura por figura](docs/21_validacao_figura_por_figura.md)
- [Matriz artigo → código](docs/22_matriz_artigo_para_codigo.md)
- [Checklist de reprodução](docs/09_checklist_reproducao.md)

---

## Sobre `closed_form` e `exact`

Dois modos de solução das equações transcendentais aparecem em todo o repositório:

- **`closed_form`** — fórmulas algébricas aproximadas de Marcatili (Eqs. 12–13 e 22–23); válidas para modos bem guiados.
- **`exact`** — solução numérica por bisseção das equações transcendentais do modelo (Eqs. 6–7 e 20–21).

> `exact` **não** significa resolver o problema vetorial 2D completo. Significa resolver numericamente, de forma exata, a equação transcendental do modelo separável implementado.

Leitura complementar: [docs/11_closed_form_vs_exact.md](docs/11_closed_form_vs_exact.md)

---

## Contribuição e desenvolvimento

Ver [DEVELOPMENT.md](DEVELOPMENT.md) para convenções de código, entradas/saídas, roteiro de evolução e o que evitar.

Para a lista de tarefas abertas e exercícios de exploração, ver [TODO.md](TODO.md).
