# 20. Auditoria técnica do modelo

Este documento faz uma leitura crítica do que o repositório `marcatili-1969-rectangular-waveguide` realmente implementa hoje.

O objetivo não é apenas julgar se “funciona”, mas separar com clareza:

- o que está fisicamente bem alinhado com Marcatili (1969);
- o que é aproximação controlada;
- o que é convenção numérica do repositório;
- o que ainda deve ser tratado como hipótese aberta.

## Resumo executivo

O repositório já resolve, de forma coerente e didática:

1. o guia retangular único;
2. o limite de lâmina;
3. o caso com interface metalizada ou de baixa impedância;
4. o acoplador direcional em forma normalizada.

O ponto mais importante da auditoria é este:

- o solver `exact` é um solver numérico das equações transcendentais do **modelo adotado**;
- ele **não** é uma solução vetorial completa do problema 2D sem as aproximações de Marcatili.

Essa distinção está cientificamente correta e, na verdade, é uma qualidade do repositório: ela permite separar o erro do modelo físico do erro adicional da forma fechada.

## Notação física mínima

Para acompanhar o restante do texto, vale fixar o papel das variáveis principais:

- $k_x$ e $k_y$: números de onda transversais dentro do núcleo;
- $k_z$: constante de propagação axial;
- $A_v$: escala geométrica associada ao contraste entre $n_1$ e $n_v$;
- $\xi_v$ e $\eta_v$: profundidades de penetração nas regiões externas.

Fisicamente:

- $k_x$ e $k_y$ quantizam como o campo “cabe” nas direções transversais;
- $k_z$ mede como o modo se propaga ao longo de $z$;
- $A_v$ controla a escala de decaimento evanescente;
- $\xi_v$ e $\eta_v$ dizem quão longe o campo penetra no dielétrico vizinho.

No artigo, para a família $E^y_{pq}$, isso aparece em Eq. (3), Eq. (6), Eq. (7), Eq. (8), Eq. (9) e Eq. (10). Para a família $E^x_{pq}$, o análogo aparece em Eq. (17), Eq. (18), Eq. (19), Eq. (20) e Eq. (21).

## 1. O que o repositório realmente resolve

## 1.1 Guia único retangular

O núcleo do guia único está em:

- `src/physics/single_guide.cpp`

O repositório resolve:

- a família $E^y_{pq}$ com `closed_form` e `exact`;
- a família $E^x_{pq}$ com `closed_form` e `exact`;
- cálculo de $k_z$;
- cálculo de $\xi_{3,5}$ e $\eta_{2,4}$;
- checagens de guiamento e de domínio.

Do ponto de vista físico, esta é a parte mais sólida da implementação atual. Ela segue diretamente a estrutura das Seções III.A e III.B do artigo.

## 1.2 Limite de lâmina

O limite de lâmina está em:

- `src/physics/slab_guide.cpp`

Ele reutiliza a estrutura do guia único, mas reduz o problema a uma única quantização transversal efetiva. Isso é adequado quando o painel da figura ou a linha da tabela deve ser lido como limite planar.

Do ponto de vista didático, essa implementação é boa porque preserva a mesma estrutura de resultado do guia retangular, facilitando comparação.

## 1.3 Guia com interface metalizada

O caso da Fig. 8 está em:

- `src/physics/metal_guide.cpp`

O repositório trata a interface superior como uma condição de fase equivalente a uma parede idealizada, adaptando o formalismo do guia único para um problema com fronteira especial.

Aqui a implementação está funcional e pedagogicamente honesta, mas é menos “fechada” cientificamente do que o guia único, porque:

- a leitura do scan ainda não resolve completamente a identificação modal intermediária;
- a interpretação visual da figura continua parcialmente dependente de OCR.

## 1.4 Acoplador direcional normalizado

O núcleo do acoplador está em:

- `src/physics/coupler.cpp`

Hoje o repositório resolve principalmente a forma normalizada da Eq. (34), usando:

- Eq. (6) e Eq. (12) no baseline de Fig. 10;
- Eq. (20) no baseline de Fig. 11.

Isso é suficiente para:

- gerar as curvas de acoplamento normalizado;
- comparar `closed_form` com `exact` no nível da raiz transversal;
- estudar a dependência exponencial com a separação.

Ainda não é, porém, um solver completo de supermodos do acoplador com saída dimensional abrangente para:

- $K$;
- $L$;
- casos perturbados da Seção V.

## 2. O que `exact` significa no contexto do projeto

`Exact` significa:

$$
\text{resolver numericamente a equação transcendental do modelo de Marcatili adotado}.
$$

Na prática, isso quer dizer:

- resolver Eq. (6) e Eq. (7) por bisseção na família $E^y_{pq}$;
- resolver Eq. (20) e Eq. (21) por bisseção na família $E^x_{pq}$;
- reaproveitar essa mesma lógica no acoplador, em variáveis normalizadas.

Arquivos centrais:

- `src/math/root_finding.cpp`
- `src/physics/single_guide.cpp`
- `src/physics/slab_guide.cpp`
- `src/physics/metal_guide.cpp`
- `src/physics/coupler.cpp`

O que `exact` **não** significa:

- solução vetorial rigorosa do problema eletromagnético 2D completo;
- tratamento explícito das regiões de canto negligenciadas por Marcatili;
- solução por FEM, FDM ou método espectral completo.

Essa limitação não é um defeito escondido. Ela está hoje razoavelmente bem documentada e deve continuar sendo ensinada assim.

## 3. O que `closed_form` significa

`Closed_form` significa:

$$
\text{usar as aproximações algébricas explícitas obtidas a partir das transcendentes}.
$$

Para a família $E^y_{pq}$, isso corresponde ao bloco Eq. (12) a Eq. (16). Para a família $E^x_{pq}$, ao bloco Eq. (22) a Eq. (26).

Fisicamente, a ideia é:

- o modo está bem guiado;
- as quantidades $\left(\frac{k_x A}{\pi}\right)^2$ e $\left(\frac{k_y A}{\pi}\right)^2$ são pequenas;
- as fases de reflexão podem ser expandidas;
- a transcendental vira uma fórmula explícita.

Em outras palavras, a forma fechada funciona melhor quando:

- o confinamento é forte;
- a potência fica majoritariamente na região 1;
- não estamos colados ao cutoff.

## 4. Adequação física ao artigo

## 4.1 Onde a adequação é forte

O alinhamento com o artigo é forte em:

- estrutura modal do guia único;
- distinção entre famílias $E^y_{pq}$ e $E^x_{pq}$;
- uso de $A_v$, $\xi_v$ e $\eta_v$;
- cálculo de $k_z$ a partir de Eq. (3) e Eq. (17);
- uso do nomograma da Fig. 7 como construção geométrica derivada das equações fechadas;
- uso da Eq. (34) como base operacional das curvas de acoplamento.

Também é forte a separação entre:

- solver físico;
- parser de entrada;
- serialização;
- scripts de plot.

Isso facilita revisão científica.

## 4.2 Onde a adequação é moderada, mas consciente

Há regiões em que o repositório não “fecha a questão”, mas registra a hipótese:

- Fig. 8: ramo modal intermediário;
- Fig. 10: coerência entre rótulo modal e referência a Eq. (6)/(12);
- Tabela I: interpretação final da grandeza tabulada;
- Fig. 8: símbolo de eixo $A$ versus $A_4$.

Isso é um bom sinal metodológico. Melhor uma hipótese marcada do que uma falsa certeza.

## 5. Adequação numérica da implementação atual

## 5.1 Pontos fortes

Do ponto de vista numérico, a implementação atual é boa para o estágio do projeto:

- bisseção é robusta para as transcendentes monotônicas usadas;
- há checagem explícita de domínio antes de aceitar a raiz;
- há distinção entre `below_cutoff` e `outside_*_domain`;
- as grandezas não finitas são tratadas com cuidado nos relatórios;
- a saída em `JSON` e `CSV` é coerente com o fluxo de plot.

Além disso, o repositório já possui:

- `10` smoke tests via `CTest`;
- scripts de build e reprodução automatizados;
- checklist de reprodução em `docs/09_checklist_reproducao.md`.

## 5.2 Limites atuais da validação numérica

O principal limite atual não é a robustez do solver, mas o tipo de teste disponível.

Hoje a cobertura é majoritariamente:

- smoke test de execução;
- comparação visual;
- conferência qualitativa de curvas.

Ainda faltam, de forma explícita:

- regressões quantitativas por figura;
- tolerâncias numéricas por curva ou por ponto de referência;
- critérios automáticos de aceitação física para a Tabela I;
- validação numérica mais forte do acoplador dimensional.

## 5.3 Significado da queda exponencial no acoplamento

Um ponto pedagógico importante é o fator exponencial das Fig. 10 e Fig. 11.

As curvas caem com a separação porque o campo no meio 5 é evanescente. Logo, o acoplamento entre os guias depende de quão forte esse campo ainda está na região do guia vizinho.

Esquematicamente:

$$
\text{acoplamento} \propto \exp\left(-\frac{c}{\xi_5}\right).
$$

Na forma normalizada usada no repositório, isso aparece em Eq. (34) como:

$$
2u^2\sqrt{1-u^2}\exp\left(-\pi\frac{c}{A_5}\sqrt{1-u^2}\right),
\qquad
u=\frac{k_x A_5}{\pi}.
$$

Por isso a separação $c/a$ altera tanto as curvas: ela age diretamente sobre a sobreposição evanescente.

## 6. Coerência entre documentação, JSON, código e saídas

O repositório hoje tem boa coerência estrutural entre:

- documentação em `docs/`;
- casos-base em `data/input/`;
- execução em `src/apps/`;
- parse/serialização em `src/io/`;
- física em `src/physics/`;
- figuras em `scripts/plot_fig*.py`.

Exemplos particularmente bons de rastreabilidade:

- `solver_model = closed_form | exact`;
- `transverse_equation = eq6 | eq20` no acoplador;
- `table_entry_interpretation` explícito na Tabela I;
- `article_target` e `ocr_note` presentes em entradas importantes.

Essa coerência é uma das partes mais sólidas do projeto hoje.

## 7. Onde o modelo é forte e onde ele é frágil

## 7.1 Forte

O modelo é mais forte em:

- regime bem guiado;
- leitura estrutural das duas famílias modais;
- reprodução qualitativa e semi-quantitativa das figuras;
- interpretação de $A_v$, $\xi_v$ e $\eta_v$;
- comparação entre solver `closed_form` e solver `exact`.

## 7.2 Frágil

O modelo é mais frágil em:

- vizinhança de cutoff;
- casos em que o pequeno parâmetro assintótico deixa de ser pequeno;
- leitura OCR de figuras com rótulos ruins;
- uso do acoplador fora do escopo normalizado atual;
- fechamento editorial da Tabela I.

Em particular, perto do cutoff:

- pequenas diferenças em $k_x$ e $k_y$ deslocam bastante $k_z$;
- a distinção entre “modo quase guiado” e “modo abaixo do cutoff” fica mais sensível;
- a forma fechada tende a degradar antes do solver transcendental.

## 8. Conclusão da auditoria

O repositório já está tecnicamente maduro o suficiente para:

- ensino da formulação aproximada de Marcatili;
- estudo de dispersão modal em guia retangular;
- comparação entre aproximação assintótica e solução transcendental do modelo;
- reprodução plausível das figuras centrais do artigo.

Ele ainda não está maduro o suficiente para afirmar, sem ressalvas, que:

- resolve o problema vetorial 2D completo;
- encerra todas as ambiguidades editoriais do artigo;
- possui validação quantitativa fechada figura a figura.

Essa é uma posição cientificamente adequada.

## Leituras relacionadas

- [11_closed_form_vs_exact.md](11_closed_form_vs_exact.md)
- [12_trilha_equacoes_para_codigo.md](12_trilha_equacoes_para_codigo.md)
- [13_validacao_e_limites_do_modelo.md](13_validacao_e_limites_do_modelo.md)
- [21_validacao_figura_por_figura.md](21_validacao_figura_por_figura.md)

