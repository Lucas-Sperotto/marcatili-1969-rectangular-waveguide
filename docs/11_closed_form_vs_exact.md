# 11. `closed_form` vs `exact`

Esta é provavelmente a distinção mais importante para ler o repositório corretamente.

O projeto usa dois nomes de solver:

- `closed_form`
- `exact`

Esses nomes são úteis, mas podem ser mal interpretados se não forem explicados com cuidado.

## A ideia central

Em ambos os casos, o repositório ainda está dentro do **modelo aproximado de Marcatili** para o guia retangular.

Ou seja:

- o campo foi separado e simplificado como no artigo;
- as equações transversais foram reduzidas a relações transcendentais 1D;
- a potência propagada nas regiões de canto continua sendo desprezada, como no modelo.

Portanto:

- `closed_form` significa: usar as **fórmulas aproximadas em forma fechada** do artigo;
- `exact` significa: resolver **numericamente** as **equações transcendentais do próprio modelo de Marcatili**.

Não significa:

- solução rigorosa do problema vetorial 2D completo;
- solução por elementos finitos;
- solução sem aproximações físicas.

## O que é `closed_form`

No solver `closed_form`, o repositório usa as fórmulas explícitas derivadas a partir das equações transcendentais no regime de guiamento forte.

Para a família $E^y_{pq}$, isso corresponde principalmente a:

- Eq. (12)
- Eq. (13)
- Eq. (14)
- Eq. (15)
- Eq. (16)

Para a família $E^x_{pq}$, isso corresponde principalmente a:

- Eq. (22)
- Eq. (23)
- Eq. (24)
- Eq. (25)
- Eq. (26)

Essas fórmulas surgem quando quantidades adimensionais pequenas permitem expandir as funções $\tan^{-1}$, como indicado por hipóteses do tipo

$$
\left(\frac{k_x A_{3,5}}{\pi}\right)^2 \ll 1,
\qquad
\left(\frac{k_y A_{2,4}}{\pi}\right)^2 \ll 1.
$$

No código, isso aparece principalmente em:

- [src/physics/single_guide.cpp](../src/physics/single_guide.cpp)
- [src/physics/slab_guide.cpp](../src/physics/slab_guide.cpp)
- [src/physics/metal_guide.cpp](../src/physics/metal_guide.cpp)
- [src/physics/coupler.cpp](../src/physics/coupler.cpp)

## O que é `exact`

No solver `exact`, o repositório não usa uma nova física. Ele usa a **mesma física de Marcatili**, mas calcula $k_x$ e $k_y$ por solução numérica das equações transcendentais.

Para a família $E^y_{pq}$:

- Eq. (6)
- Eq. (7)
- Eq. (8)
- Eq. (9)
- Eq. (10)

Para a família $E^x_{pq}$:

- Eq. (20)
- Eq. (21)
- Eq. (18)
- Eq. (19)
- Eq. (17)

O método numérico usado hoje é bisseção em 1D, implementada em:

- [src/math/root_finding.cpp](../src/math/root_finding.cpp)

Isso é robusto e suficiente para este estágio do projeto.

## Comparação direta

| solver | equações associadas | hipótese principal | custo computacional | uso no repositório | limitação |
| --- | --- | --- | --- | --- | --- |
| `closed_form` | Eq. (12)-(16), Eq. (22)-(26), Eq. (12)/(22) dentro do acoplador | modo bem guiado; expansão assintótica das relações transcendentais | baixo | sweeps rápidos, Fig. 6, Fig. 7, comparação com `exact`, parte de Fig. 10 | perde fidelidade quando os pequenos parâmetros deixam de ser pequenos |
| `exact` | Eq. (6)-(10), Eq. (17)-(21), Eq. (20) no acoplador | mesma física de Marcatili, mas sem substituir a transcendental por forma fechada | moderado | validação, comparação, Fig. 8, Fig. 10, Fig. 11, Table I | continua sendo exato apenas dentro do modelo aproximado do artigo |

## Onde cada um aparece

### Guia único

- `SolveSingleGuideClosedForm(...)` em [src/physics/single_guide.cpp](../src/physics/single_guide.cpp)
- `SolveSingleGuideExact(...)` em [src/physics/single_guide.cpp](../src/physics/single_guide.cpp)

### Limite de lâmina

- `SolveSlabGuideClosedForm(...)` em [src/physics/slab_guide.cpp](../src/physics/slab_guide.cpp)
- `SolveSlabGuideExact(...)` em [src/physics/slab_guide.cpp](../src/physics/slab_guide.cpp)

### Caso metalizado da Fig. 8

- `SolveMetalGuideClosedForm(...)` em [src/physics/metal_guide.cpp](../src/physics/metal_guide.cpp)
- `SolveMetalGuideExact(...)` em [src/physics/metal_guide.cpp](../src/physics/metal_guide.cpp)

### Acoplador

- `SolveClosedFormKxRatio(...)` em [src/physics/coupler.cpp](../src/physics/coupler.cpp)
- `SolveExactKxRatio(...)` em [src/physics/coupler.cpp](../src/physics/coupler.cpp)
- `SolveCouplerPoint(...)` em [src/physics/coupler.cpp](../src/physics/coupler.cpp)

## O que o leitor deve ter em mente

Se você vier de métodos numéricos gerais de eletromagnetismo, a palavra `exact` pode sugerir algo como:

- solução modal vetorial completa;
- cálculo rigoroso das condições de contorno em toda a seção;
- tratamento explícito das regiões de canto negligenciadas por Marcatili.

**Não é isso que o repositório quer dizer.**

Aqui, `exact` quer dizer:

$$
\text{solução numérica da equação transcendental do modelo adotado}.
$$

Isso é cientificamente honesto e útil, porque separa duas fontes de erro:

1. erro do **modelo físico**;
2. erro adicional da **aproximação em forma fechada**.

Essa separação é exatamente o que permite comparar:

- fidelidade do artigo;
- domínio de validade das fórmulas assintóticas;
- comportamento dos gráficos reproduzidos.

## Resumo prático

Se a pergunta for “qual solver usar para estudar a estrutura do artigo?”, a regra prática é:

- use `closed_form` para entender a álgebra e gerar rapidamente as curvas aproximadas;
- use `exact` para verificar quanto da diferença vem apenas da substituição algébrica das transcendentes.

## Leituras relacionadas

- [10_fluxo_geral_do_repositorio.md](10_fluxo_geral_do_repositorio.md)
- [12_trilha_equacoes_para_codigo.md](12_trilha_equacoes_para_codigo.md)
- [13_validacao_e_limites_do_modelo.md](13_validacao_e_limites_do_modelo.md)

