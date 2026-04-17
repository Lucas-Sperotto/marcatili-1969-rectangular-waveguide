# 13. Validação e limites do modelo

Este documento resume o que o repositório já consegue afirmar com segurança e onde ainda precisamos manter cautela editorial ou científica.

## O que já foi validado

Hoje o repositório já mostra comportamento plausível e rastreável para:

- curvas de dispersão da Fig. 6;
- nomograma da Fig. 7;
- caso metalizado da Fig. 8;
- curvas de acoplamento das Fig. 10 e Fig. 11;
- busca numérica da Tabela I.

“Validado”, aqui, não significa “concordância perfeita pixel a pixel com o scan”.

Significa, nesta fase:

1. ordem modal plausível;
2. monotonicidade coerente;
3. escalas normalizadas consistentes com o artigo;
4. fronteiras do gráfico compatíveis com o painel original;
5. uso explícito das equações corretas, ou de uma hipótese marcada quando a leitura do scan é ambígua.

## O que já apresenta boa concordância visual

Com base no relatório de divergências em [00.4_relatorio_divergencias_figuras.md](00.4_relatorio_divergencias_figuras.md):

- Fig. 6: comportamento geral bom, com alguns ajustes finos de borda e agrupamento;
- Fig. 7: nomograma numericamente coerente;
- Fig. 8: tendência geral e regime de guiamento plausíveis;
- Fig. 10: tendência global boa após correções de escala e legenda;
- Fig. 11: estrutura das famílias e aproximação à degenerescência bem capturadas.

## Divergência física vs divergência editorial

Vale separar dois tipos de diferença.

### Divergência física

É quando a matemática produz um comportamento possivelmente diferente do artigo, por exemplo:

- ordem das curvas incompatível;
- cutoff em região errada;
- família modal suspeita;
- dependência com parâmetros em desacordo com o texto.

### Divergência editorial

É quando a física parece correta, mas a apresentação ainda difere do impresso:

- legenda em caixa em vez de rótulos sobre as curvas;
- moldura e grade diferentes;
- ausência do esquema geométrico desenhado dentro do painel;
- ocupação de borda diferente do scan.

Essa distinção é importante porque evita gastar esforço “consertando física” quando o problema é só de composição gráfica.

## Limites importantes do modelo

## 1. O solver `exact` não é o problema 2D completo

O solver `exact` resolve exatamente as equações transcendentais do **modelo aproximado de Marcatili**.

Ele não resolve:

- o problema vetorial completo em 2D;
- as regiões de canto com um método rigoroso;
- um cálculo de supermodos completo por elementos finitos ou diferenças finitas.

Essa distinção está detalhada em [11_closed_form_vs_exact.md](11_closed_form_vs_exact.md).

## 2. O parser JSON é intencionalmente restrito

O parser interno atende bem o schema atual do repositório, mas não deve ser tratado como parser JSON geral.

Isso é um limite de infraestrutura, não de física. Mesmo assim, ele importa para reprodutibilidade:

- casos novos devem preferir objetos JSON explícitos;
- esquemas compactos em string devem ser evitados no longo prazo.

## 3. Fig. 8 ainda tem uma ambiguidade modal aberta

O caso-base atual de Fig. 8 acompanha o conjunto de trabalho:

- $E^y_{11}$
- $E^x_{11}$
- $E^x_{21}$

Essa escolha é plausível e operacional, mas o scan ainda não é totalmente inequívoco.

Status correto:

- **não é bug confirmado**;
- **não é questão encerrada**;
- deve permanecer como `TODO OCR`.

## 4. Fig. 10 continua com ambiguidade histórica/documental

A Seção IV cita explicitamente Eq. (6) e Eq. (12) ao introduzir a Fig. 10, mas o rótulo modal do scan ainda é OCR-ambíguo.

O repositório escolheu uma postura correta:

- seguir conscientemente a referência textual do artigo;
- registrar a ambiguidade, em vez de escondê-la.

Ainda permanece aberta a revisão:

- do rótulo modal da figura;
- da família intermediária que visualmente pode sugerir `1.6` no scan.

## 5. Tabela I continua sendo sensível editorialmente

O ponto mais delicado da Tabela I é a interpretação da grandeza tabulada.

Hoje o schema deixa explícito:

$$
\texttt{table\_entry\_interpretation} = \texttt{"a\_times\_n1\_over\_lambda"}.
$$

Isso é melhor do que deixar a hipótese escondida, mas ainda não elimina a necessidade de conferência com o original.

Portanto:

- o tratamento atual é honesto e rastreável;
- a questão científica-editorial ainda não deve ser considerada encerrada.

## O que significa “boa concordância”

Neste projeto, “boa concordância” deve significar:

1. as curvas aparecem na ordem certa;
2. os ramos principais entram e saem nas regiões esperadas;
3. a inclinação geral e o regime assintótico batem com o artigo;
4. os eixos normalizados estão consistentes;
5. a diferença entre `closed_form` e `exact` é coerente com o que o artigo sugere.

Boa concordância **não** exige:

- reproduzir imperfeições do scan;
- reproduzir a tipografia original;
- esconder hipóteses onde o artigo está ilegível.

## Onde olhar quando surgir uma divergência

Se uma reprodução parecer errada, a ordem recomendada de inspeção é:

1. confirmar o caso em `data/input/*.json`;
2. confirmar o solver escolhido (`closed_form` ou `exact`);
3. confirmar as equações esperadas no campo `equations_used`;
4. confirmar se a divergência é física ou apenas editorial;
5. checar os relatórios:
   - [00.4_relatorio_divergencias_figuras.md](00.4_relatorio_divergencias_figuras.md)
   - [00.5_avaliacao_plano_fases.md](00.5_avaliacao_plano_fases.md)
   - [00.6_revisao_camada_io.md](00.6_revisao_camada_io.md)

## Assuntos que permanecem abertos

- `TODO OCR` da Fig. 8;
- `TODO OCR` da Fig. 10;
- refinamento editorial das bordas e rótulos da Fig. 6;
- fechamento definitivo da interpretação tabulada da Tabela I;
- futura amarração dimensional completa de $K$ e $L$ no acoplador.

## Perguntas Frequentes sobre o Modelo

#### P: O que exatamente significa o solver `exact` neste projeto?

**R:** `exact` significa que estamos resolvendo **numericamente as equações transcendentais do modelo simplificado de Marcatili** (e.g., Eq. 6 e 7). Ele é "exato" *dentro desse modelo*. Ele **não** é uma solução rigorosa das equações de Maxwell para a geometria 2D completa, pois ainda se baseia na aproximação de Marcatili de ignorar os campos nos cantos do guia.

*Leitura recomendada: 11_closed_form_vs_exact.md.*

---

#### P: Por que o solver `closed_form` diverge do `exact` perto do corte (cutoff)?

**R:** O solver `closed_form` (forma fechada) é derivado sob a aproximação de "modo bem guiado". Matematicamente, isso permite usar `tan⁻¹(z) ≈ z`, que só é válido para `z` pequeno. Perto do corte, o modo está fracamente confinado, o campo evanescente se espalha muito, e o argumento `z` da função `tan⁻¹` não é mais pequeno. A aproximação falha, e as fórmulas algébricas perdem a precisão.

---

#### P: Por que a Fig. 10 usa a Eq. (6) e a Fig. 11 usa a Eq. (20) para o cálculo de `kx`?

**R:** Porque elas se referem a famílias de modos diferentes, que têm polarizações e condições de contorno distintas.
-   A **Fig. 10** analisa o acoplamento para modos da família $E^x_{pq}$ (no texto, embora a implementação siga a referência à Eq. 6, que é da família $E^y_{pq}$ para a direção x - uma ambiguidade histórica do artigo).
-   A **Fig. 11** analisa o acoplamento para modos da família $E^y_{pq}$ (no texto, embora a implementação use a Eq. 20, que é da família $E^x_{pq}$ para a direção x).

A implementação atual segue a interpretação mais consistente com a estrutura do artigo, onde as equações para `kx` e `ky` trocam de forma dependendo da polarização dominante. A aparente troca de equações nas figuras é uma das ambiguidades documentadas do artigo original.

---

#### P: A Tabela I está comparando a largura `a` ou a altura `b`?

**R:** Esta é uma excelente pergunta e um ponto de interpretação em aberto. A hipótese de trabalho atual no repositório é que o valor tabelado (multiplicado por `λ/n₁`) corresponde à **largura `a`**. Essa hipótese é baseada em exemplos numéricos e na consistência com outras partes do artigo. No entanto, isso não é uma certeza absoluta e está marcado como um ponto que requer validação final.

---

#### P: O repositório resolve o problema vetorial completo sem aproximações?

**R:** **Não.** Todo o repositório se baseia no **modelo aproximado de Marcatili**. A principal simplificação é desprezar a energia que se propaga nas regiões de canto do guia (as áreas hachuradas na Fig. 3 do artigo). Isso permite que o problema seja separado em duas direções (x e y), o que o torna analiticamente tratável. Uma solução "completa" exigiria métodos numéricos mais complexos, como Elementos Finitos (FEM) ou Diferenças Finitas (FDM), que estão fora do escopo deste projeto.

---

#### P: Por que existem casos para `slab` (lâmina) e `metal`?

**R:** Eles representam casos especiais ou limites importantes discutidos no próprio artigo.
-   **`slab`:** Corresponde ao limite de um guia de onda de lâmina (planar), que ocorre quando uma das dimensões transversais (e.g., `a`) se torna muito grande (`a → ∞`). É usado para reproduzir os painéis (d) e (k) da Fig. 6.
-   **`metal`:** Corresponde ao caso da Fig. 8, onde uma das interfaces do guia é substituída por uma parede metálica (ou de baixa impedância). O artigo explora isso como uma forma de remover a degenerescência de polarização e forçar a operação em um único modo.

## Mensagem principal

O repositório já está forte o bastante para estudo, reprodução e evolução incremental, mas ainda preserva explicitamente as regiões onde o artigo não está legível o suficiente para encerrar a discussão.

Isso é uma qualidade científica do projeto, não uma fraqueza.
