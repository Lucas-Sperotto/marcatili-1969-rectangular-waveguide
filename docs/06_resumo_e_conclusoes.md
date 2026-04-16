# VI. Resumo e conclusões

Uma haste dielétrica (Fig. 4a), de seção transversal retangular $a \times b$, circundada por diferentes dielétricos, suporta, por meio de reflexão interna total, duas famílias de modos híbridos. Esses modos são essencialmente modos do tipo TEM, polarizados nas direções $x$ ou $y$; nós os denominamos $E^{x}_{pq}$ e $E^{y}_{pq}$. Os índices $p$ e $q$ indicam o número de extremos ( $p$ na direção $x$ e $q$ na direção $y$ ) das componentes transversais do campo elétrico ou magnético.

Curvas de dispersão para guias com diferentes proporções e diferentes dielétricos circundantes são apresentadas nas Figs. 6a até 6k. Dimensões típicas para vários guias capazes de suportar apenas os modos fundamentais $E^{x}_{11}$ e $E^{y}_{11}$ estão reunidas na Tabela I.

Ao escolher dielétricos com índices semelhantes, as dimensões do guia podem ser grandes em comparação com $\lambda$, reduzindo assim as exigências de tolerância. As dimensões $a$ e $b$ podem ser escolhidas arbitrariamente e ainda assim obter-se um guia que suporte apenas os modos fundamentais, se os índices de refração puderem ser escolhidos adequadamente. O projeto é realizado com o auxílio da equação (14) ou da Fig. 7.

A desvantagem que se paga, na maioria desses guias, é que os modos fundamentais são quase degenerados; consequentemente, imperfeições de simetria tendem a acoplá-los. Uma camada com perdas, adicionada à interface $y=b/2$ (Fig. 4a), deve atenuar o modo $E^{x}_{11}$ mais do que o $E^{y}_{11}$. Como alternativa, o guia pode ser projetado para suportar apenas o modo $E^{y}_{11}$, metalizando essa mesma interface. Curvas de dispersão desse caso são mostradas na Fig. 8.

Como o campo não fica completamente confinado, existe acoplamento entre dois desses guias (Fig. 3). Curvas de projeto para acopladores direcionais são fornecidas nas Figs. 10 e 11.

Tipicamente, para

$$
n_1=1.5,
\qquad
n_2=n_3=n_4=n_5=\frac{1.5}{1.01},
\qquad
a=3.54\lambda,
\qquad
b=\frac{a}{2}=1.77\lambda,
\qquad
c=\frac{a}{4}=0.88\lambda,
$$

de acordo com a equação (33), o comprimento necessário para acoplamento de 3 dB é $L/2=131\lambda$. Esse comprimento aumenta exponencialmente com a separação entre os guias.

Aumentar o índice de refração entre os guias em apenas três partes por mil dobra o acoplamento.

Qual é uma separação razoável para evitar acoplamento? Usando os valores do exemplo anterior, dois guias paralelos de 1 cm de comprimento, separados por 2,5 vezes a largura de cada guia, apresentam um acoplamento de $-40\,\text{dB}$.

Os guias dielétricos e os acopladores direcionais descritos mostram grande potencial como elementos básicos para circuitos ópticos integrados porque:

1. podem ser feitos monomodo mesmo quando suas dimensões transversais são grandes em comparação com o comprimento de onda no espaço livre de operação; consequentemente, as exigências de tolerância podem ser relaxadas;
2. permitem a construção de componentes ópticos compactos;
3. são mecanicamente estáveis, e os problemas de alinhamento são minimizados;
4. são estruturas relativamente simples e se prestam à fabricação com alta precisão por técnicas de circuitos integrados;
5. podem incluir dispositivos ativos de dimensões comparavelmente pequenas.

## Fig. 13 — Comportamento qualitativo do coeficiente de acoplamento em função de $h$

Legenda:  
**Fig. 13 —** Comportamento qualitativo do coeficiente de acoplamento em função de $h$.

---

## Observações editoriais

- Nesta conclusão, Marcatili retoma os pontos centrais do artigo: modos híbridos, curvas de dispersão, condição monomodo, quase degenerescência dos modos fundamentais e aplicação em acopladores direcionais.
- O termo **single mode** foi traduzido como **monomodo**.
- Mantive a distinção entre a vantagem estrutural do guia e a limitação associada à quase degenerescência dos modos fundamentais.
- O trecho final é claramente programático: ele posiciona essas estruturas como candidatas naturais para óptica integrada.

## Comentário técnico complementar

Esta conclusão consolida muito bem o valor do artigo. O principal mérito de Marcatili está em mostrar que uma estrutura relativamente simples, uma haste dielétrica retangular cercada por meios de menor índice, pode ser analisada de forma aproximada, mas ainda assim útil para projeto. A teoria não pretende substituir uma solução rigorosa em todos os regimes, mas oferece fórmulas, curvas e critérios de dimensionamento bastante práticos.

Do ponto de vista do repositório, esta seção praticamente define o checklist final de reprodução:

- reproduzir as curvas de dispersão;
- reproduzir as condições de operação monomodo;
- mostrar a quase degenerescência dos modos fundamentais;
- reproduzir o caso com metalização de interface;
- reproduzir as curvas de acoplamento;
- validar os exemplos numéricos de separação e comprimento de acoplador.

Também fica claro que a contribuição do artigo não é apenas teórica, mas também tecnológica: ele fornece argumentos concretos para o uso desses guias em circuitos ópticos integrados compactos e fabricáveis.

## Texto original correspondente

- Seção **VI. Summary and Conclusions**;
- retomada dos principais resultados teóricos e práticos do artigo;
- lista final de vantagens dos guias dielétricos e acopladores direcionais para óptica integrada.
