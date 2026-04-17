# Inventario da raiz

Este arquivo documenta a pasta raiz do repositorio. O objetivo e deixar explicito onde cada parte do projeto fica e como a arvore principal se organiza.

Escopo adotado nesta documentacao: a arvore versionada do projeto. Diretorios internos de ferramenta, como `.git/`, e diretorios gerados localmente, como `build/`, nao entram na regra de `readme.md` por subpasta.

## Subpastas do projeto

- `data/`: arquivos de entrada e saida usados pelos executaveis e pelos scripts de reproducao.
- `docs/`: documentacao tecnica, traducao comentada do artigo e material de apoio; esta pasta foi explicitamente excluida do pedido atual.
- `include/`: cabecalhos publicos do nucleo em C++17.
- `scripts/`: scripts externos de plotagem, automacao e verificacao.
- `src/`: implementacao reutilizavel e executaveis do projeto.

## Diretorios auxiliares fora do escopo desta politica

- `.git/`: metadados locais do Git, mantidos pela ferramenta de versionamento.
- `build/`: artefatos locais de configuracao, compilacao e teste gerados pelo CMake/CTest.

## Arquivos da raiz

- `.gitignore`: regras de arquivos e diretorios que nao devem ser rastreados pelo Git.
- `AGENTS.md`: instrucoes de trabalho especificas deste repositorio para agentes de desenvolvimento.
- `CMakeLists.txt`: configuracao principal de build, biblioteca `marcatili_core`, executaveis e smoke tests.
- `LICENSE`: arquivo de licenca do repositorio.
- `README.md`: visao geral do projeto, motivacao, mapa artigo -> codigo e comandos principais.
- `readme.md`: este inventario resumido da pasta raiz.
