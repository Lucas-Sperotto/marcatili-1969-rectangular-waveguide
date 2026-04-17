# 24. Plano de melhoria priorizado

Este plano organiza os próximos passos em fases, pensando em:

- impacto científico;
- clareza pedagógica;
- esforço de implementação;
- risco de retrabalho.

O objetivo não é “fazer tudo ao mesmo tempo”, mas criar uma sequência que preserve a coerência do repositório.

## Resumo de prioridades

A ordem recomendada é:

1. manter fechados os bloqueios técnicos e a validação automática;
2. fechar ambiguidades OCR centrais ainda abertas;
3. refinar apresentação e fac-símile;
4. expandir o escopo do acoplador a partir da base dimensional já disponível.

## Fase 1: documentação e rastreabilidade

### Status

**concluída**

### Objetivo

Fechar a base documental necessária para que ninguém confunda:

- hipótese aberta com fato consolidado;
- solver `exact` com solução vetorial completa;
- divergência editorial com divergência física.

### Arquivos-alvo

- `docs/20_auditoria_tecnica_do_modelo.md`
- `docs/21_validacao_figura_por_figura.md`
- `docs/22_matriz_artigo_para_codigo.md`
- `docs/23_riscos_tecnicos_e_pendencias.md`
- `docs/24_plano_de_melhoria_priorizado.md`
- `docs/00.1_figuras.md`
- `docs/00.2_casos_de_teste.md`
- `docs/02_symbol_dictionary.md`

### Esforço estimado

**baixo a médio**

### Risco

**baixo**, desde que a fase continue evitando refatorações grandes.

### Critério de conclusão

- toda ambiguidade central está explicitamente registrada;
- existe uma ponte clara artigo $\to$ código $\to$ caso-base;
- o leitor novo sabe o que o repositório resolve e o que ainda não resolve.
- a Tabela I já expõe `table_entry_interpretation` e `cutoff_status`.

## Fase 2: validação numérica e checks automáticos

### Status

**concluída**

### Objetivo

Passar de “o executável roda” para “o comportamento numérico esperado está protegido”.

### Arquivos-alvo

- `CMakeLists.txt`
- `tests/regression_checks.cpp`
- `data/input/*.json`
- `data/output/` de referência, quando apropriado

### Esforço estimado

**médio**

### Risco

**médio**, porque tolerâncias mal escolhidas podem gerar falsos alarmes ou cristalizar uma hipótese ainda não fechada.

### Critério de conclusão

- regressões quantitativas para pontos de referência das figuras;
- regressões quantitativas para Tabela I;
- cobertura explícita da diferença entre `closed_form` e `exact` em casos representativos;
- documentação das tolerâncias adotadas.
- proteção explícita dos estados `below_cutoff`, `below_transverse_cutoff`, `below_search_min` e `above_search_max`.

## Fase 3: refinamento das figuras e fac-símile

### Status

**aberta**

### Objetivo

Melhorar a comparação visual sem confundir isso com correção física.

### Arquivos-alvo

- `scripts/plot_fig6.py`
- `scripts/plot_fig7.py`
- `scripts/plot_fig8.py`
- `scripts/plot_fig10.py`
- `scripts/plot_fig11.py`
- `scripts/compare_fig*_article.py`
- scripts `scripts/run/build_fig*_article_comparison.sh`

### Esforço estimado

**médio**

### Risco

**médio**, porque é fácil gastar muito tempo imitando o scan sem ganho científico equivalente.

### Critério de conclusão

- rótulos mais próximos do artigo;
- bordas e limites por painel revisados;
- montagens artigo $\times$ reprodução completas;
- distinção documentada entre versão “didática limpa” e versão “article_style”.

## Fase 4: expansão do acoplador e casos perturbados

### Status

**parcial**

### Objetivo

Expandir o repositório do acoplamento normalizado atual para uma cobertura mais completa da Seção IV e do caso de guias ligeiramente diferentes.

### Arquivos-alvo

- `src/physics/coupler.cpp`
- `src/apps/solve_coupler.cpp`
- `src/io/coupler_io.cpp`
- `data/input/solve_coupler.json`
- novos casos em `data/input/`
- documentação do acoplador em `docs/04_acoplador_direcional.md`
- documentação da Seção V em `docs/05_Acoplador direcional construído com guias ligeiramente diferentes.md`

### Esforço estimado

**médio a alto**

### Risco

**alto**, porque a expansão do acoplador exige fechar melhor normalizações, exemplos numéricos e, idealmente, referências externas.

### Critério de conclusão

- `solve_coupler` produz saída dimensional consistente para $K$ e $L$;
- exemplos numéricos do texto são reproduzidos de forma rastreável;
- há plano claro para o caso perturbado da Seção V.

## Ordem prática sugerida dentro das fases

Se o projeto continuar em iterações curtas, a sequência recomendada é:

1. fechar Fig. 8;
2. fechar Fig. 10;
3. incorporar referência externa de Jones/Goell;
4. só então investir pesado em fac-símile visual;
5. depois expandir o acoplador para casos perturbados.

## O que não fazer cedo demais

Algumas coisas parecem tentadoras, mas não são a melhor próxima ação:

- refatorar fortemente a arquitetura do C++;
- trocar o parser JSON por uma solução mais pesada sem necessidade imediata;
- perseguir fac-símile visual completo antes de fechar ambiguidades científicas;
- vender `exact` como se fosse solver vetorial rigoroso.

## Conclusão

O repositório já está numa fase em que a melhoria mais valiosa não é “reescrever tudo”, mas sim:

- reduzir incerteza onde ela realmente importa;
- proteger os comportamentos corretos com validação automática;
- só depois polir apresentação e ampliar escopo.

Esse é o caminho com melhor relação entre esforço, risco e ganho científico.


<!-- NAV START -->
---

**Navegação:** [Anterior](23_riscos_tecnicos_e_pendencias.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](25_dossie_de_fechamento_das_pendencias_centrais.md)

<!-- NAV END -->
