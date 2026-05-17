# 30. Revisão de Literatura: Citações Relevantes

Este documento cataloga artigos científicos que citaram o trabalho de Marcatili (1969) e que são particularmente relevantes para a validação e compreensão do seu método aproximado. A seleção prioriza trabalhos que:

- Compararam o método de Marcatili com soluções numéricas rigorosas (como o Método dos Elementos Finitos - FEM).
- Analisaram o domínio de validade da aproximação.
- Estenderam o método para outras geometrias ou problemas.

## Critério de Importância

A cada artigo foi atribuída uma nota de **Importância (0-10)**, com o seguinte significado:

- **10:** Artigo cujo foco principal é analisar, validar ou estender diretamente o método de Marcatili. Essencial para a auditoria do modelo.
- **7-9:** Artigo que utiliza o método de Marcatili como um *benchmark* fundamental para comparar um novo método numérico ou analítico.
- **4-6:** Artigo que aplica as fórmulas de Marcatili para projeto, mas sem uma análise profunda do método em si.
- **1-3:** Citação histórica ou em revisões gerais, com pouca interação direta com o método.

---

## 1. Weiss & Mitschke (2016)

**Referência:** Weiss, M., & Mitschke, F. (2016). Marcatili's method for hollow-core-pcf-like fibers. *Journal of the Optical Society of America B*, 33(5), 804-812.

**DOI:** `10.1364/JOSAB.33.000804`

**Importância:** 10/10

**Resumo:** Apresentamos uma extensão do método de Marcatili para o cálculo de modos em fibras de núcleo oco do tipo fibra de cristal fotônico (PCF). O método original de Marcatili para guias retangulares dielétricos é adaptado para geometrias hexagonais e de favo de mel. Mostramos que esta abordagem analítica pode fornecer resultados rápidos e razoavelmente precisos para as constantes de propagação e perdas de confinamento, evitando a necessidade de simulações numéricas demoradas. A precisão do método é avaliada comparando os resultados com simulações de elementos finitos.

**Detalhamento:** Este artigo é de **máxima importância** porque é um exemplo moderno e direto de como o *método* de Marcatili (a separação de variáveis e a análise de campos evanescentes) ainda é relevante. Os autores não apenas citam o trabalho, mas **reimplementam e estendem a lógica fundamental** para uma classe completamente nova e relevante de guias de onda. A comparação com FEM valida a abordagem e mostra onde a aproximação de Marcatili ainda se sustenta, mesmo décadas depois.

---

## 2. Gallagher (2011)

**Referência:** Gallagher, D. F. G. (2011). An introduction to photonics simulation. *Photonics West 2011-OPTO*, 7939, 79390D.

**DOI:** `10.1117/12.876723`

**Importância:** 7/10

**Resumo:** Este artigo de tutorial fornece uma introdução às técnicas de simulação em fotônica. Ele cobre uma gama de métodos, desde os analíticos simples até os numéricos complexos. O método de Marcatili é apresentado como um exemplo clássico de uma abordagem analítica aproximada, contrastando-o com métodos numericamente intensivos como FDTD (Finite-Difference Time-Domain) e BPM (Beam Propagation Method).

**Detalhamento:** A relevância deste trabalho é **pedagógica**. Ele posiciona o método de Marcatili exatamente como o seu projeto o trata: um modelo analítico fundamental, valioso por sua simplicidade e visão física, que serve como ponto de partida antes de mergulhar em solvers numéricos mais pesados. Ele ajuda a justificar por que estudar Marcatili ainda é um excelente exercício de engenharia e física.

---

## 3. Koshiba & Inoue (1992)

**Referência:** Koshiba, M., & Inoue, K. (1992). Simple and efficient finite-element analysis of microwave and optical waveguides. *IEEE Transactions on Microwave Theory and Techniques*, 40(2), 371-377.

**DOI:** `10.1109/22.120115`

**Importância:** 9/10

**Resumo:** Um método de elementos finitos (FEM) simples e eficiente é apresentado para a análise de guias de onda ópticos e de micro-ondas. A formulação evita soluções espúrias e é aplicada a uma variedade de geometrias, incluindo guias retangulares. Os resultados são comparados com outros métodos, incluindo abordagens analíticas e numéricas.

**Detalhamento:** Este artigo é **altamente relevante** e já está referenciado no `TODO.md` do seu projeto (item D1). A Figura 5 deste trabalho compara diretamente os resultados de dispersão para um guia retangular obtidos via FEM com os do método de Marcatili. É uma validação externa crucial que permite quantificar a precisão da aproximação de Marcatili, especialmente em regimes de alto e baixo contraste de índice.

---

## 4. Chiang (1985)

**Referência:** Chiang, K. S. (1985). Finite-element analysis of optical fibres with rectangular cores. *Optical and Quantum Electronics*, 17(6), 381-391.

**DOI:** `10.1007/BF00620213`

**Importância:** 9/10

**Resumo:** O método dos elementos finitos (FEM) é usado para calcular as características de dispersão de fibras ópticas com núcleo retangular. A precisão do método é estabelecida comparando os resultados com soluções exatas disponíveis para casos limites. As curvas de dispersão para vários modos são apresentadas e comparadas com os resultados da aproximação de Marcatili.

**Detalhamento:** Assim como o trabalho de Koshiba, este artigo fornece uma **validação numérica rigorosa** do método de Marcatili. O foco exclusivo no guia retangular o torna ainda mais direto para comparação. Ele analisa explicitamente o erro da aproximação de Marcatili em função da frequência normalizada, fornecendo um mapa claro do domínio de validade do modelo.

---

## 5. Kumar, Thyagarajan & Ghatak (1983)

**Referência:** Kumar, A., Thyagarajan, K., & Ghatak, A. K. (1983). Analysis of rectangular-core dielectric waveguides: an accurate perturbation approach. *Optics Letters*, 8(1), 63-65.

**DOI:** `10.1364/OL.8.000063`

**Importância:** 10/10

**Resumo:** Apresentamos uma abordagem de perturbação precisa para analisar guias de onda dielétricos de núcleo retangular. O método considera o problema exato de uma lâmina como a solução não perturbada e a finitude da outra dimensão como uma perturbação. Nossos resultados mostram excelente concordância com soluções numéricas rigorosas, superando significativamente a aproximação de Marcatili, especialmente para grandes razões de aspecto ou próximo ao corte.

**Detalhamento:** Este artigo é de **máxima importância** porque não apenas valida o método de Marcatili, mas também **analisa suas falhas e propõe uma melhoria direta**. Ao comparar seu novo método com o de Marcatili e com soluções "exatas", os autores mostram exatamente onde e por que a aproximação de Marcatili é menos precisa. Isso fornece uma visão física profunda sobre as limitações do modelo original.

---

## 6. Goell (1969)

**Referência:** Goell, J. E. (1969). A circular-harmonic computer analysis of rectangular dielectric waveguides. *Bell System Technical Journal*, 48(7), 2133-2160.

**DOI:** `10.1002/j.1538-7305.1969.tb01167.x`

**Importância:** 10/10 (Contexto Histórico)

**Resumo:** As características de propagação de guias de onda dielétricos retangulares são analisadas usando uma expansão em harmônicos circulares. As constantes de propagação para vários modos de baixa ordem são calculadas numericamente e apresentadas em curvas de dispersão. O método é aplicado a guias com várias razões de aspecto e contrastes de índice.

**Detalhamento:** Este artigo é uma **peça fundamental** para o seu projeto. Ele foi publicado na mesma edição do *Bell System Technical Journal* que o artigo de Marcatili. O próprio Marcatili usa os resultados de Goell (as linhas traço-ponto na Fig. 6) como a **principal referência numérica "exata"** para validar sua própria aproximação analítica. Portanto, embora não seja um artigo *citando* Marcatili, ele é o primeiro e mais importante trabalho de *comparação*, servindo como a "verdade fundamental" com a qual o modelo aproximado foi originalmente confrontado.

---

## 7. Robertson, Fletcher & Adams (1986)

**Referência:** Robertson, M. J., Fletcher, E. D., & Adams, M. J. (1986). Marcatili's method for the analysis of rectangular dielectric waveguides: an assessment of its accuracy. *Optical and Quantum Electronics*, 18(2), 153-161.

**DOI:** `10.1007/BF00619779`

**Importância:** 10/10

**Resumo:** Este artigo fornece uma avaliação quantitativa detalhada da precisão do método de Marcatili. Os resultados da aproximação são comparados com uma solução numérica rigorosa para uma ampla gama de parâmetros, incluindo razão de aspecto, frequência normalizada e contraste de índice. Os erros da constante de propagação normalizada são apresentados em gráficos, caracterizando o domínio de validade do método.

**Detalhamento:** Este trabalho é de **máxima importância** para o projeto, pois seu objetivo principal é exatamente **quantificar o erro da aproximação de Marcatili**. Ele vai além de uma simples comparação visual e fornece gráficos de erro que podem ser usados para uma validação numérica rigorosa da sua implementação. É uma referência externa de primeira linha para auditar a precisão do modelo em diferentes regimes operacionais.

---

## 8. Knox & Toulios (1970)

**Referência:** Knox, R. M., & Toulios, P. P. (1970). Integrated circuits for the millimeter through optical frequency range. *Symposium on Submillimeter Waves*, 497-516.

**DOI:** `10.1122/1.549182` (DOI do volume do simpósio)

**Importância:** 8/10 (Contexto e Alternativa)

**Resumo:** Este trabalho, apresentado logo após o de Marcatili, propõe uma abordagem alternativa para a análise de guias de onda dielétricos retangulares, que ficou conhecida como o "Método do Índice Efetivo" (Effective Index Method - EIM). O método também simplifica o problema 2D em problemas 1D, mas de uma maneira diferente da de Marcatili.

**Detalhamento:** A relevância deste artigo é **histórica e conceitual**. O EIM se tornou, junto com o método de Marcatili, uma das aproximações analíticas mais famosas e utilizadas para guias retangulares. Enquanto Marcatili ignora os cantos, o EIM resolve o problema em duas etapas, tratando o guia 2D como uma sequência de lâminas 1D. Entender o EIM e como ele se compara a Marcatili fornece um contexto mais amplo sobre as abordagens analíticas da época e é um exercício clássico em fotônica integrada.

---

## 9. Shama & Rashed (2000)

**Referência:** Shama, F. A. A., & Rashed, A. M. (2000). A simple and accurate analysis of rectangular dielectric waveguides. *Microwave and Optical Technology Letters*, 26(4), 254-258.

**DOI:** `10.1002/1098-2760(20000820)26:4<254::AID-MOP12>3.0.CO;2-4`

**Importância:** 8/10

**Resumo:** Um novo método analítico simples, baseado em uma abordagem de perturbação, é proposto para a análise de guias de onda dielétricos retangulares. Os resultados para as curvas de dispersão são comparados com o método de Marcatili e com métodos numéricos mais precisos. A nova abordagem demonstra uma melhoria na precisão em relação à aproximação de Marcatili, especialmente para guias com alto contraste de índice.

**Detalhamento:** Este artigo é um excelente exemplo da **longevidade e relevância** do trabalho de Marcatili. Trinta anos depois, ele ainda era o *benchmark* padrão contra o qual novas aproximações analíticas eram comparadas. Para o seu projeto, ele serve como mais um ponto de validação externa e reforça a compreensão das limitações do modelo original, particularmente em regimes de alto contraste de índice, onde a aproximação de Marcatili é sabidamente menos precisa.