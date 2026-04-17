# 23. Riscos técnicos e pendências

Este documento reúne os principais riscos atuais do projeto.

A ideia é evitar dois extremos:

- tratar tudo como “quase resolvido”, quando ainda há pontos científicos abertos;
- tratar tudo como “instável”, quando várias partes já estão sólidas.

## Escala usada

Impacto:

- **alto**: pode alterar a interpretação física principal ou comprometer uma reprodução central;
- **médio**: não destrói o projeto, mas reduz confiabilidade ou rastreabilidade;
- **baixo**: afeta mais acabamento do que a conclusão científica.

Urgência:

- **alta**: deveria entrar nas próximas rodadas;
- **média**: importante, mas pode esperar uma etapa posterior;
- **baixa**: bom de resolver, porém não bloqueia ensino nem evolução.

## Matriz de riscos

| risco | descrição | impacto | urgência | sugestão de ação |
| --- | --- | --- | --- | --- |
| R1 | ambiguidade modal da Fig. 8 | alto | alta | rever scan, revisar texto do artigo e testar famílias concorrentes mantendo `TODO OCR` explícito |
| R2 | família intermediária da Fig. 10 (`1.0` vs `1.6`) | alto | alta | gerar comparação controlada com ambas as famílias e congelar a leitura adotada com justificativa |
| R3 | coerência entre rótulo modal da Fig. 10 e referência a Eq. (6)/(12) | alto | alta | manter hipótese explícita, registrar decisão editorial e buscar confirmação adicional no original |
| R4 | solver `exact` pode ser confundido com solução 2D rigorosa | médio | média | continuar reforçando em docs e comentários que `exact` é exato apenas dentro do modelo de Marcatili |
| R5 | parser JSON restrito por projeto | médio | média | manter documentação explícita do escopo do parser e evitar crescer o schema com sintaxes compactas frágeis |
| R6 | presença residual de campos compactados em string em alguns casos antigos | médio | média | migrar casos canônicos restantes para arrays de objetos JSON e deixar strings compactas apenas como compatibilidade legado |
| R7 | falta de referência externa digitalizada para Jones/Goell | médio | média | incorporar dados ou referências rastreáveis para reforçar validação do acoplador |
| R8 | diferenças entre fac-símile visual e concordância física | médio | média | documentar sempre se a divergência é científica ou apenas editorial antes de mexer no plot |
| R9 | ajuste fino de alguns painéis da Fig. 6 ainda incompleto | médio | média | revisar painéis `6c`, `6e` e `6j` com critérios por painel |
| R10 | eixo da Fig. 8 ainda pode ser $A$ ou $A_4$ | baixo | média | confirmar notação final no scan e congelar terminologia nos scripts e docs |
| R11 | interpretação do limite de lâmina em alguns painéis pode ser supercarregada visualmente | baixo | baixa | simplificar apresentação final dos painéis de lâmina quando o foco for fac-símile |

## Comentário por grupo

## Riscos científicos principais

Os riscos mais importantes são:

- R1: Fig. 8;
- R2 e R3: Fig. 10;
- R7: referência externa de Jones/Goell.

Esses riscos merecem prioridade porque podem mudar não apenas o “desenho” final da figura, mas a interpretação física que o repositório transmite.

## Riscos de infraestrutura

Os riscos de infraestrutura mais relevantes são:

- R5: parser JSON minimalista;
- R6: formatos compactos legados.

Aqui o risco não é tanto “matemática errada”, mas sim:

- crescimento difícil do repositório;
- manutenção mais frágil;
- menor confiança em refinos futuros.

## Riscos editoriais

Há também riscos editoriais que não devem ser confundidos com falhas físicas:

- R8: fac-símile visual versus concordância física;
- R9: bordas e agrupamentos de Fig. 6;
- R10: notação de eixo em Fig. 8;
- R11: apresentação do limite de lâmina.

Esses pontos importam, mas devem ser tratados depois que os riscos científicos principais estiverem mais fechados.

## Pendências concretas

Além dos riscos, há pendências de desenvolvimento já bem delimitadas:

1. fechar a leitura OCR da Fig. 8;
2. fechar a leitura OCR da Fig. 10;
3. incorporar uma referência rastreável para Jones/Goell;
4. migrar casos canônicos remanescentes para arrays de objetos JSON;
5. decidir se o próximo foco é:
   - acabamento científico das figuras;
   - ou expansão do acoplador para casos perturbados.

## Conclusão

O projeto não está num estado “frágil”. O núcleo físico principal, a distinção entre limite físico e erro de domínio, a saída dimensional básica do acoplador e a regressão quantitativa central já estão fechados.

O que resta agora é um conjunto menor e mais localizado de ambiguidades OCR, referências externas e acabamento editorial.

Isso é exatamente o que se espera de um repositório científico em maturação saudável.


<!-- NAV START -->
---

**Navegação:** [Anterior](22_matriz_artigo_para_codigo.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](24_plano_de_melhoria_priorizado.md)

<!-- NAV END -->
