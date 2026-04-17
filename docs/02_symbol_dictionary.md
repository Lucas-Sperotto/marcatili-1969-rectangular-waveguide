# 02 Symbol Dictionary

Este arquivo consolida os simbolos usados no repositorio para evitar ambiguidades entre documentacao, codigo C++ e scripts Python.

## Convencoes gerais

- Modos da familia `E_y` usam notacao sem negrito no artigo.
- Modos da familia `E_x` aparecem em negrito no artigo original; no codigo usamos os mesmos nomes sem negrito.
- Sempre que houver duvida de OCR, registrar `TODO OCR` no arquivo de caso (`data/input/*.json`) e no relatorio correspondente.

## Geometria

| Simbolo | Significado | Unidade |
| --- | --- | --- |
| $a$ | meia-largura eletrica/parametro horizontal do guia (largura do nucleo no eixo $x$) | m |
| $b$ | meia-altura eletrica/parametro vertical do guia (altura do nucleo no eixo $y$) | m |
| $c$ | separacao horizontal entre os dois guias no acoplador direcional | m |
| $p$ | indice modal na direcao $x$ | inteiro |
| $q$ | indice modal na direcao $y$ | inteiro |
| $\lambda$ | comprimento de onda no vacuo | m |

## Indices de refracao

| Simbolo | Significado |
| --- | --- |
| $n_1$ | indice do nucleo do guia |
| $n_2$ | indice do meio externo na face inferior |
| $n_3$ | indice do meio externo na face lateral esquerda |
| $n_4$ | indice do meio externo na face superior |
| $n_5$ | indice do meio externo na face lateral direita |
| $r$ | razao usada nas equacoes do acoplador para Eq. (20): $r=\left(\frac{n_5}{n_1}\right)^2$ |

## Numeros de onda

| Simbolo | Definicao |
| --- | --- |
| $k_0$ | numero de onda no vacuo: $k_0=\frac{2\pi}{\lambda}$ |
| $k_i$ | numero de onda no meio $i$: $k_i=k_0 n_i$ |
| $k_x$ | numero de onda transversal no eixo $x$ |
| $k_y$ | numero de onda transversal no eixo $y$ |
| $k_z$ | numero de onda axial de propagacao no guia unico |

No guia unico:

$$
k_z^2 = k_1^2 - k_x^2 - k_y^2.
$$

## Parametros auxiliares $A_i$

Para cada meio externo $i \in \{2,3,4,5\}$:

$$
A_i=\frac{\lambda}{2\sqrt{n_1^2-n_i^2}}.
$$

No repositorio aparecem como `A2`, `A3`, `A4`, `A5`.

## Profundidades de penetracao

Para familia `E_y` (Secao III do artigo):

$$
\xi_{3,5}=\left[\left(\frac{\pi}{A_{3,5}}\right)^2-k_x^2\right]^{-1/2},
\qquad
\eta_{2,4}=\left[\left(\frac{\pi}{A_{2,4}}\right)^2-k_y^2\right]^{-1/2}.
$$

No codigo, `xi3`, `xi5`, `eta2`, `eta4`.

## Familias modais

| Nome no codigo | Nome no artigo | Observacao |
| --- | --- | --- |
| `E_y` | $E^y_{pq}$ | primeira familia hibrida, componente dominante associada a $E_y$ |
| `E_x` | $E^x_{pq}$ | segunda familia hibrida, componente dominante associada a $E_x$ |

## Normalizacoes usadas nas figuras

### Figura 6

Eixo horizontal:

$$
X_6=\frac{b}{A_4}=\frac{2b}{\lambda}\sqrt{n_1^2-n_4^2}.
$$

Eixo vertical:

$$
Y_6=\frac{k_z^2-k_4^2}{k_1^2-k_4^2}.
$$

### Figura 7 (nomograma)

$$
X=\left(\frac{\pi}{a}\right)^2
\left(1+\frac{A_3+A_5}{\pi a}\right)^{-2}
\left(k_1^2-k_z^2\right)^{-1},
$$

$$
Y=\left(\frac{\pi}{b}\right)^2
\left(1+\frac{n_2^2A_2+n_4^2A_4}{\pi n_1^2 b}\right)^{-2}
\left(k_1^2-k_z^2\right)^{-1},
$$

com retas modais:

$$
p^2X+q^2Y=1.
$$

E retas auxiliares:

$$
\frac{Y}{X}=C.
$$

### Figura 8

Eixo horizontal:

$$
X_8=\frac{a}{A}\approx \frac{2a}{\lambda}\sqrt{n_1^2-n_4^2}.
$$

Eixo vertical:

$$
Y_8=\frac{k_z^2-k_4^2}{k_1^2-k_4^2}.
$$

`TODO OCR`: confirmar no scan final se o simbolo de eixo e $A$ ou $A_4$.

### Figuras 10 e 11

Eixo horizontal:

$$
\frac{c}{a}.
$$

Parametro de familia:

$$
\frac{a}{A_5}=\frac{2a}{\lambda}\sqrt{n_1^2-n_5^2}.
$$

Saida normalizada da Eq. (34), usada no codigo como `normalized_coupling`:

$$
2u^2\sqrt{1-u^2}\exp\left(-\pi\frac{c}{A_5}\sqrt{1-u^2}\right),
\quad
u=\frac{k_xA_5}{\pi}.
$$

## Notacao operacional no codigo

- `solver_model = closed_form | exact`
- `transverse_equation = eq6 | eq20` (acoplador)
- `mode_indices.p`, `mode_indices.q`
- `normalized_geometry.a_over_A5`, `normalized_geometry.c_over_a`

## Ambiguidades abertas (OCR)

- Rotulos de alguns modos na Fig. 8 (familia intermediaria).
- Coerencia entre a descricao textual da Fig. 10 e a referencia de equacoes (6)/(12).
- Confirmacao final de notacao de eixo na Fig. 8 ($A$ vs $A_4$).
