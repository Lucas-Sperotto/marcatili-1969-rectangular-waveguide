# 09 Checklist de Reproducao

Este checklist formaliza os passos minimos para reproduzir os artefatos numericos e graficos do repositorio.

## Checklist rapido

1. Limpar, compilar e reproduzir:

```bash
./scripts/run.sh full
```

2. Verificar artefatos obrigatorios:

```bash
./scripts/run.sh check
```

## Itens de aceitacao por fase

- `build/` gerado sem erro de compilacao.
- Executaveis principais executam com entrada por arquivo JSON.
- Saidas numericas em `data/output/*.json` e `data/output/*.csv`.
- Graficos base em `data/output/*.png` para Figs. 7, 8, 10 e 11.
- Paineis da Fig. 6 em `data/output/fig6/*.png`, com rotulos `(a)`, `(b)`, ...
- Tabela I em `reproduce_table1.summary.csv` e `reproduce_table1.details.csv`.

## Riscos conhecidos ainda abertos

- Ambiguidades OCR de legenda/modal nas Figs. 8 e 10.
- Referencias externas (Goell/Jones) ainda sem digitalizacao completa para comparacao numerica automatica.
- Cobertura de testes ainda majoritariamente de smoke (faltam regressos quantitativos de curvas).


<!-- NAV START -->
---

**Navegação:** [Anterior](refs/README.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](10_fluxo_geral_do_repositorio.md)

<!-- NAV END -->
