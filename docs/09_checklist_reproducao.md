# 09 Checklist de Reproducao

Este checklist formaliza os passos minimos para reproduzir os artefatos numericos e graficos do repositorio.

## Checklist rapido

1. Limpar, compilar e reproduzir:

```bash
./scripts/run/clean_build_reproduce_all.sh
```

2. Verificar artefatos obrigatorios:

```bash
./scripts/run/check_reproduction.sh
```

3. (Opcional) Rodar a verificacao completa em um unico comando:

```bash
RUN_PIPELINE=1 ./scripts/run/check_reproduction.sh
```

## Itens de aceitacao por fase

- `build/` gerado sem erro de compilacao.
- Executaveis principais executam com entrada por arquivo JSON.
- Saidas numericas em `data/output/*.json` e `data/output/*.csv`.
- Graficos base em `data/output/*.png` para Figs. 6, 7, 8, 10 e 11.
- Tabela I em `reproduce_table1.summary.csv` e `reproduce_table1.details.csv`.
- Comparacoes visuais disponiveis quando scripts `build_fig*_article_comparison.sh` forem executados.

## Riscos conhecidos ainda abertos

- Ambiguidades OCR de legenda/modal nas Figs. 8 e 10.
- Referencias externas (Goell/Jones) ainda sem digitalizacao completa para comparacao numerica automatica.
- Cobertura de testes ainda majoritariamente de smoke (faltam regressos quantitativos de curvas).
