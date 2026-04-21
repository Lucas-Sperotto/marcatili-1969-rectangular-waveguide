# data/output/

Esta pasta concentra os artefatos gerados pelos executáveis e pelos scripts de plotagem.

## Artefatos por executável

| Executável | Artefatos produzidos |
| --- | --- |
| `solve_single_guide` | `solve_single_guide.json`, `solve_single_guide.csv` |
| `solve_coupler` | `solve_coupler.json`, `solve_coupler.csv` |
| `reproduce_fig6` | `fig6/SG-006*.json`, `fig6/SG-006*.csv`, `fig6/SG-006*.png` |
| `reproduce_fig7` | `reproduce_fig7.json`, `reproduce_fig7.lines.csv`, `reproduce_fig7.intersections.csv`, `reproduce_fig7.png` |
| `reproduce_fig8` | `reproduce_fig8.json`, `reproduce_fig8.csv`, `reproduce_fig8.png` |
| `reproduce_fig10` | `reproduce_fig10.json`, `reproduce_fig10.csv`, `reproduce_fig10.png` |
| `reproduce_fig11` | `reproduce_fig11.json`, `reproduce_fig11.csv`, `reproduce_fig11.png` |
| `reproduce_table1` | `reproduce_table1.json`, `reproduce_table1.summary.csv`, `reproduce_table1.details.csv` |

## Notas

- Saídas numéricas são sempre `CSV` ou `JSON`; imagens `PNG` são geradas pelos scripts Python.
- O subdiretório `fig6/` concentra os painéis individuais da Fig. 6.
- Para regenerar todos os artefatos: `./scripts/run.sh reproduce`.
