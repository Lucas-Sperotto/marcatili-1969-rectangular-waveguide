# Contributing

Obrigado por considerar uma contribuicao para este repositorio.

O objetivo do projeto e reproduzir, com clareza, rastreabilidade e fidelidade, o artigo de Marcatili (1969) sobre guia dieletrico retangular e acoplador direcional. Por isso, contribuicoes tecnicamente corretas e cientificamente bem documentadas sao mais valiosas do que mudancas grandes ou rapidas.

## Antes de contribuir

- Leia o [README.md](README.md) para entender o escopo atual do repositorio.
- Consulte `docs/` para verificar o estado da traducao, das equacoes e das pendencias tecnicas.
- Se a mudanca afetar regras operacionais para agentes automatizados, leia tambem [AGENTS.md](AGENTS.md).

## Principios do projeto

- Priorize clareza, didatica e fidelidade ao artigo original.
- Nao invente formulas ausentes. Quando houver lacuna, registre a incerteza explicitamente.
- Marque ambiguidades de OCR, traducao ou notacao com `TODO`.
- Mantenha a separacao entre fisica, matematica, I/O e executaveis.
- Preserve o nucleo numerico em `C++17`, sem dependencias desnecessarias.
- Garanta que toda saida numerica continue indo para `CSV` ou `JSON`.
- Gere figuras apenas por scripts externos em `scripts/`.
- Faca cada executavel aceitar um arquivo de entrada.
- Registre, sempre que possivel, quais equacoes, figuras ou tabelas do artigo cada modulo implementa.

## Estrutura esperada

- `src/apps/`: pontos de entrada executaveis.
- `src/` e `include/`: implementacao reutilizavel.
- `data/input/`: casos de entrada.
- `data/output/`: resultados gerados e artefatos de reproducao.
- `scripts/`: automacao e plotagem externa.
- `docs/`: documentacao tecnica, traducao e auditoria.

## Estilo de edicao

- Use o arquivo [`.editorconfig`](.editorconfig) como base de indentacao, newline e whitespace.
- Prefira comentarios curtos e tecnicos, especialmente em trechos numericos ou fisicos menos obvios.
- Em arquivos Markdown, use `$...$` para matematica inline e `$$...$$` para blocos.

## Build e verificacao

Os comandos principais do fluxo atual sao:

```bash
./scripts/run.sh build
./scripts/run.sh reproduce
./scripts/run.sh check
RUN_TESTS=1 ./scripts/run.sh build
```

Se a sua mudanca mexer na camada numerica, em parsing, ou nos executaveis, rode ao menos o build e os smoke tests.

## Mudancas cientificas e numericas

Ao alterar formulas, criterios de guiamento, interpretacoes de OCR ou parametros dos casos:

- explique no pull request quais equacoes ou figuras foram afetadas;
- diga se a mudanca altera resultados em `data/output/`;
- explicite se a mudanca corrige bug, refina aproximacao ou muda interpretacao editorial;
- inclua `TODO` quando ainda houver dependencia de verificacao no artigo ou no scan.

## Artefatos gerados

Este repositorio versiona parte dos artefatos de reproducao em `data/output/`. Nao trate essa pasta como lixo descartavel por padrao: se a sua mudanca altera resultados canonicos, atualize os artefatos relevantes de forma consciente e descreva isso no review.

## Pull requests

Um bom pull request aqui costuma conter:

- objetivo claro e escopo limitado;
- referencia ao trecho do artigo, figura ou tabela afetada;
- resumo do impacto numerico ou documental;
- observacoes sobre riscos, limites ou pendencias restantes.

## Sobre `AGENTS.md`

`AGENTS.md` deve ser mantido enquanto o repositorio continuar usando agentes automatizados ou fluxos assistidos por IA. Ele complementa este guia com restricoes e prioridades especificas para automacao; nao substitui o `README.md` nem este `CONTRIBUTING.md`.
