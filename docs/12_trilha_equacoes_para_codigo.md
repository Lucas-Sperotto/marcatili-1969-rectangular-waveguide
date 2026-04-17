# 12. Trilha das equações para o código

Um dos princípios fundamentais deste repositório é a **rastreabilidade** entre a teoria de Marcatili e o código C++. Este documento é uma ponte direta entre o artigo e a implementação, servindo como um mapa que conecta as equações às suas implementações.

## A Estratégia Central de Marcatili

O problema de um guia dielétrico retangular é complexo. A genialidade de Marcatili foi simplificá-lo com duas aproximações principais:

1.  **Desprezar os campos nos cantos:** A análise considera apenas as regiões 1 a 5 da Fig. 3, ignorando as regiões hachuradas. Isso permite que as equações de Maxwell sejam separáveis nas variáveis `x` e `y`.
2.  **Contraste de índice fraco:** A suposição de que os índices de refração do núcleo (`n1`) e do revestimento (`n2` a `n5`) são próximos (`(n1/n_v) - 1 << 1`) implica que os modos são bem guiados e quase-TEM.

Isso transforma um problema de valor de contorno 2D complexo em dois problemas 1D independentes, um para cada direção transversal (`x` e `y`).

## Do Guia Único ao Código

O comportamento de um modo é governado por suas constantes de propagação: `kx` e `ky` (transversais) e `kz` (axial). Elas estão ligadas pela **relação de dispersão**. O desafio é encontrar `kx` e `ky`. O código implementa as duas abordagens descritas no artigo: a solução numérica das equações transcendentais (`exact`) e a aproximação em forma fechada (`closed_form`).

### Relação de Dispersão (Eq. 3 e 17)

$$
k_z = \left(k_1^2-k_x^2-k_y^2\right)^{1/2}
$$

Esta é a espinha dorsal do cálculo de propagação axial para a família $E^y_{pq}$.

No código, ela aparece em:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): `SolveSingleGuideClosedForm(...)` e `SolveSingleGuideExact(...)`
- [src/physics/slab_guide.cpp](../src/physics/slab_guide.cpp): limite de lâmina
- [src/physics/metal_guide.cpp](../src/physics/metal_guide.cpp): versão adaptada para Fig. 8

Observação:

- no `closed_form`, $k_x$ e $k_y$ já vêm das fórmulas explícitas;
- no `exact`, $k_x$ e $k_y$ são obtidos numericamente antes de usar a mesma relação.

## Eq. (6) e Eq. (7)

Essas são as equações transcendentais da família $E^y_{pq}$.

No código:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): lambdas `fx` e `fy` em `SolveSingleGuideExact(...)`
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): `SolveExactKxRatio(...)` no ramo `kEq6`, já em forma normalizada para o acoplador

Observação:

- no acoplador, a Eq. (6) é reescrita em termos de
  $$
  u = \frac{k_x A_5}{\pi},
  $$
  porque a Eq. (34) é expressa naturalmente nessa variável.

## Eq. (8), Eq. (9) e Eq. (10)

Essas equações introduzem profundidades de penetração e a escala $A_v$.

No código:

- [src/math/waveguide_math.cpp](../src/math/waveguide_math.cpp): `ComputeA(...)` e `PenetrationDepth(...)`
- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): cálculo de $\xi_{3,5}$ e $\eta_{2,4}`
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): substituição de $\xi_5$ na forma normalizada de Eq. (34)

Essas rotinas são um bom exemplo de separação entre:

- física do artigo;
- utilitário numérico reaproveitável.

## Eq. (12) e Eq. (13)

Estas são as fórmulas `closed_form` da família $E^y_{pq}$ para $k_x$ e $k_y$.

No código:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): ramo `SingleGuideFamily::kEy` em `SolveSingleGuideClosedForm(...)`
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): `SolveClosedFormKxRatio(...)` no ramo `kEq6`, já reescrita no limite simétrico $n_3=n_5$

## Eq. (14), Eq. (15) e Eq. (16)

Estas são as expressões explícitas para:

- $k_z$
- $\xi_{3,5}$
- $\eta_{2,4}$

na família $E^y_{pq}$, depois de substituir Eq. (12) e Eq. (13).

No código:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): continuação do ramo `E_y` em `SolveSingleGuideClosedForm(...)`

O repositório ainda acrescenta dois blocos que não pertencem diretamente ao artigo:

- checagens de domínio de validade;
- normalização de $k_z$ contra $n_4$ para geração das figuras.

## Eq. (17)

$$
\mathbf{k}_z = \left(k_1^2-\mathbf{k}_x^2-\mathbf{k}_y^2\right)^{1/2}
$$

É o análogo de Eq. (3) para a família $E^x_{pq}$.

No código:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): ramo `E_x` de `SolveSingleGuideExact(...)` e `SolveSingleGuideClosedForm(...)`
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): entra indiretamente na interpretação física do ramo `kEq20`

## Eq. (18), Eq. (19), Eq. (20) e Eq. (21)

Estas são as profundidades de penetração e as transcendentes da família $E^x_{pq}$.

No código:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): ramo `E_x` de `SolveSingleGuideExact(...)`
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): `SolveExactKxRatio(...)` no ramo `kEq20`
- [src/physics/fig11.cpp](../src/physics/fig11.cpp): Fig. 11 fixa conscientemente `transverse_equation = Eq. (20)`

## Eq. (22), Eq. (23), Eq. (24), Eq. (25) e Eq. (26)

Estas são as fórmulas `closed_form` da família $E^x_{pq}$.

No código:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp): ramo `E_x` de `SolveSingleGuideClosedForm(...)`
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): `SolveClosedFormKxRatio(...)` no ramo `kEq20`

## Eq. (27) a Eq. (30): nomograma da Fig. 7

O repositório materializa o nomograma como três peças:

1. constantes geométrico-materiais;
2. linhas modais no plano $(X,Y)$;
3. linhas de projeto $Y=CX$.

No código:

- [src/physics/fig7.cpp](../src/physics/fig7.cpp): `x_numerator`, `y_numerator` e `derived_c`
- [src/physics/fig7.cpp](../src/physics/fig7.cpp): `ModeLineY(...)`
- [src/physics/fig7.cpp](../src/physics/fig7.cpp): `CLineY(...)`

Em outras palavras, a matemática da Fig. 7 não está num “solver modal” separado, mas num construtor de linhas geométricas derivadas das equações fechadas.

## Eq. (33) e Eq. (34): acoplador

Essas equações são o núcleo da reprodução do acoplador.

No código:

- [src/physics/coupler.cpp](../src/physics/coupler.cpp): `SolveCouplerPoint(...)`
- [src/physics/fig10.cpp](../src/physics/fig10.cpp): sweep de Fig. 10
- [src/physics/fig11.cpp](../src/physics/fig11.cpp): sweep de Fig. 11

Mais precisamente:

- Eq. (34) aparece na forma normalizada do campo `normalized_coupling`;
- Eq. (33) entra como contexto dimensional que motiva essa forma normalizada.

### Observação importante sobre a Fig. 10

O repositório segue conscientemente a frase da Seção IV que associa a Fig. 10 a Eq. (6) e Eq. (12), mesmo com a ambiguidade OCR do rótulo modal no scan.

Isso está marcado como assunto aberto em:

- [docs/00.1_figuras.md](00.1_figuras.md)
- [docs/00.4_relatorio_divergencias_figuras.md](00.4_relatorio_divergencias_figuras.md)
- [docs/00.6_revisao_camada_io.md](00.6_revisao_camada_io.md)

## Eq. (47) a Eq. (64): apêndice e ligação com o modelo usado

Aqui vale uma distinção importante.

O repositório **não** implementa hoje um solver completo de supermodos simétricos e antissimétricos do acoplador a partir do Apêndice A em toda a sua forma original. Em vez disso, ele usa a ponte que o próprio artigo constrói entre:

- as equações do apêndice;
- as equações simplificadas da Seção III;
- a forma prática do acoplamento na Seção IV.

### Eq. (47), Eq. (49), Eq. (50), Eq. (51), Eq. (52)

Essas equações do apêndice reapresentam, para o acoplador, as mesmas peças conceituais:

- equações transcendentais transversais;
- profundidades de penetração;
- escala $A_v$.

No código, a ligação é:

- [src/math/waveguide_math.cpp](../src/math/waveguide_math.cpp): $A_v$, profundidades de penetração
- [src/physics/coupler.cpp](../src/physics/coupler.cpp): raízes transversais normalizadas usadas em Eq. (34)

### Eq. (53) e Eq. (60)

São as expressões de $k_z$ para as duas famílias no acoplador.

No repositório atual:

- elas aparecem mais como pano de fundo conceitual do que como API explícita do acoplador;
- o foco operacional está em `normalized_coupling`, não ainda em uma saída dimensional completa de $K$ e $L$.

### Eq. (56) e Eq. (59)

Estas expressões do apêndice são particularmente importantes porque mostram a origem física do fator exponencial de acoplamento.

No código:

- [src/physics/coupler.cpp](../src/physics/coupler.cpp): o cálculo de `normalized_coupling` é a versão operacional normalizada dessa estrutura

### Eq. (61) a Eq. (64)

São o análogo para a família $E^x_{pq}$ no apêndice.

No código:

- [src/physics/coupler.cpp](../src/physics/coupler.cpp): ramo `kEq20`
- [src/physics/fig11.cpp](../src/physics/fig11.cpp): escolha do ramo da Fig. 11

## Resumo rápido

Se você quiser localizar rapidamente uma equação no código:

- Eq. (3), Eq. (14)-(16) $\rightarrow$ `single_guide.cpp`
- Eq. (6), Eq. (7) $\rightarrow$ `single_guide.cpp`, ramo `exact`
- Eq. (17), Eq. (20), Eq. (24)-(26) $\rightarrow$ `single_guide.cpp`, ramo `E_x`
- Eq. (27)-(30) $\rightarrow$ `fig7.cpp`
- Eq. (33)-(34) $\rightarrow$ `coupler.cpp`, `fig10.cpp`, `fig11.cpp`
- Eq. (47)-(64) $\rightarrow$ `waveguide_math.cpp`, `coupler.cpp`, `metal_guide.cpp`, mais a documentação do apêndice

## Leituras relacionadas

- [11_closed_form_vs_exact.md](11_closed_form_vs_exact.md)
- [13_validacao_e_limites_do_modelo.md](13_validacao_e_limites_do_modelo.md)
- [07_apendice_A.md](07_apendice_A.md)


<!-- NAV START -->
---

**Navegação:** [Anterior](11_closed_form_vs_exact.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](13_validacao_e_limites_do_modelo.md)

<!-- NAV END -->
