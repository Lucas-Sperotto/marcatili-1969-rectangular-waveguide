# Scripts

Os scripts deste diretório executam apenas tarefas externas ao núcleo em C++.

- geração de gráficos a partir de arquivos `CSV` e, quando necessário, de saídas derivadas de `JSON`;
- ferramentas auxiliares para build, reprodução e verificação dos artefatos do projeto.

## Scripts atuais

- `run.sh`: ponto único de entrada para limpeza, build, testes, reprodução e verificação dos artefatos obrigatórios;
- `plot_fig6.py`: plota um painel da Fig. 6 a partir do `CSV` gerado por `reproduce_fig6`, com limite superior do eixo `y` parametrizado por painel e opção de sobrescrita via `--y-max`;
- `plot_fig7.py`: plota o nomograma da Fig. 7 a partir dos arquivos `CSV` de linhas e interseções gerados por `reproduce_fig7`;
- `plot_fig8.py`: plota a reprodução atual da Fig. 8 a partir do `CSV` gerado por `reproduce_fig8`;
- `plot_fig10.py`: plota a reprodução atual da Fig. 10 a partir do `CSV` gerado por `reproduce_fig10`;
- `plot_fig11.py`: plota a reprodução atual da Fig. 11 a partir do `CSV` gerado por `reproduce_fig11`, com codificação explícita do solver por cor e da razão `n_1/n_5` por estilo de linha.

## Fluxo atual

- `./scripts/run.sh build`: configura, compila e, opcionalmente, executa os testes via `CTest`;
- `./scripts/run.sh fig6`: gera os painéis da Fig. 6 em `data/output/fig6/`, com identificação por letras `(a)`, `(b)`, ...;
- `./scripts/run.sh fig7`: gera a figura principal correspondente à Fig. 7 no estilo do artigo;
- `./scripts/run.sh fig8`: gera a figura principal correspondente à Fig. 8 no estilo do artigo;
- `./scripts/run.sh fig10`: gera a figura principal correspondente à Fig. 10 no estilo do artigo;
- `./scripts/run.sh fig11`: gera a figura principal correspondente à Fig. 11 no estilo do artigo;
- `./scripts/run.sh reproduce`: executa toda a reprodução numérica e gera as imagens finais do fluxo principal;
- `./scripts/run.sh check`: verifica a presença e a integridade dos artefatos obrigatórios do fluxo atual;
- `./scripts/run.sh full`: executa limpeza, build, reprodução e verificação.

## Observação de projeto

A responsabilidade destes scripts é restrita à orquestração externa do pipeline. Toda a lógica numérica, formulação física e geração dos dados-base permanece concentrada no núcleo em C++.
