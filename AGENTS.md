# AGENTS.md

## Objetivo

Reproduzir, com clareza e fidelidade, o artigo de Marcatili (1969) sobre guia dielétrico retangular e acoplador direcional.

## Princípios de desenvolvimento

1. Priorizar clareza, fidelidade ao artigo e didática.
2. Não inventar fórmulas ausentes; quando faltar informação, registrar a lacuna e pedir verificação.
3. Marcar ambiguidades de OCR com `TODO`.
4. Separar explicitamente física, matemática, I/O e executáveis.
5. Manter o núcleo numérico em `C++17`, sem dependências desnecessárias.
6. Garantir que toda saída numérica vá para `CSV` ou `JSON`.
7. Gerar todos os gráficos por scripts Python externos em `scripts/`.
8. Fazer cada executável aceitar um arquivo de entrada, evitando casos fixos no código.
9. Comentar o código de forma clara e suficiente para facilitar revisão científica.
10. Registrar, sempre que possível, quais equações, figuras ou tabelas do artigo cada módulo implementa.

## Convenções para documentação

1. Usar `$...$` para matemática inline em arquivos `.md`.
2. Usar `$$...$$` para blocos matemáticos em arquivos `.md`.
3. Preservar rastreabilidade entre texto, equações, figuras e resultados numéricos.
4. Quando houver dúvida de tradução, OCR ou notação, explicitar a incerteza em vez de ocultá-la.

## Convenções para evolução do repositório

1. Manter a estrutura principal em `docs`, `refs`, `data`, `include`, `src`, `apps`, `scripts`, `tests` e `run`.
2. Tratar `apps/` como ponto de entrada e `src/`/`include/` como implementação reutilizável.
3. Usar `data/input/` para casos de entrada e `data/output/` para resultados gerados.
4. Implementar primeiro uma base mínima, verificável e organizada; depois expandir a matemática.
