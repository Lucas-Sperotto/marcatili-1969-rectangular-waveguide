# 22. Matriz artigo para código

Esta matriz foi construída para servir como tabela de rastreabilidade entre:

- artigo;
- implementação;
- status atual;
- observações de auditoria.

| item do artigo | equação / figura | arquivo(s) do repositório | status | observação |
| --- | --- | --- | --- | --- |
| propagação axial da família $E^y_{pq}$ | Eq. (3) | `src/physics/single_guide.cpp`, `src/physics/slab_guide.cpp`, `src/physics/metal_guide.cpp` | implementado | usado como relação central para $k_z$ |
| transcendentes da família $E^y_{pq}$ | Eq. (6), Eq. (7) | `src/physics/single_guide.cpp` | implementado | resolvidas numericamente no solver `exact` |
| profundidades e escala geométrica da família $E^y_{pq}$ | Eq. (8), Eq. (9), Eq. (10) | `src/math/waveguide_math.cpp`, `src/physics/single_guide.cpp` | implementado | $A_v$, $\xi_v$ e $\eta_v$ também entram no acoplador |
| forma fechada da família $E^y_{pq}$ | Eq. (12) | `src/physics/single_guide.cpp`, `src/physics/coupler.cpp` | implementado | Eq. (12) também aparece no baseline `closed_form` da Fig. 10 |
| forma fechada da família $E^y_{pq}$ | Eq. (13) | `src/physics/single_guide.cpp`, `src/physics/slab_guide.cpp` | implementado | base para $k_y$ em `closed_form` |
| grandezas explícitas da família $E^y_{pq}$ | Eq. (14) a Eq. (16) | `src/physics/single_guide.cpp` | implementado | o código acrescenta checagens de domínio e normalização para plot |
| propagação axial da família $E^x_{pq}$ | Eq. (17) | `src/physics/single_guide.cpp` | implementado | análogo da Eq. (3) |
| profundidades da família $E^x_{pq}$ | Eq. (18), Eq. (19) | `src/physics/single_guide.cpp`, `src/physics/coupler.cpp` | implementado | usadas diretamente ou por consequência algébrica |
| transcendentes da família $E^x_{pq}$ | Eq. (20), Eq. (21) | `src/physics/single_guide.cpp`, `src/physics/coupler.cpp` | implementado | Eq. (20) é a base `exact` da Fig. 11 |
| forma fechada da família $E^x_{pq}$ | Eq. (22), Eq. (23) | `src/physics/single_guide.cpp`, `src/physics/coupler.cpp` | implementado | Eq. (22) já existe no núcleo do acoplador, mesmo que Fig. 11 baseline use apenas `exact` |
| grandezas explícitas da família $E^x_{pq}$ | Eq. (24) a Eq. (26) | `src/physics/single_guide.cpp` | implementado | usados no solver `closed_form` da família $E^x_{pq}$ |
| nomograma | Eq. (27) a Eq. (30) | `src/physics/fig7.cpp`, `src/io/fig7_io.cpp`, `data/input/reproduce_fig7.json` | implementado | a leitura OCR do modo de referência ainda tem nota aberta |
| acoplamento entre guias | Eq. (33), Eq. (34) | `src/physics/coupler.cpp`, `src/physics/fig10.cpp`, `src/physics/fig11.cpp` | implementado | forma normalizada implementada e saída dimensional adicional disponível quando `wavelength`, `n1` e `n5` são fornecidos |
| comprimento de acoplamento e uso prático | Eq. (35) | documentação e casos de exemplo do acoplador | parcial | `solve_coupler` já reporta $L$ no modelo reduzido, mas os exemplos textuais completos da Seção IV ainda pedem fechamento editorial |
| transcendentes do apêndice para o acoplador $E^y_{pq}$ | Eq. (47) a Eq. (52) | `src/math/waveguide_math.cpp`, `src/physics/coupler.cpp`, `docs/07_apendice_A.md` | parcial | usados de forma reduzida e normalizada, não como solver completo de supermodos |
| propagação axial e acoplamento do apêndice $E^y_{pq}$ | Eq. (53) a Eq. (56) | `src/physics/coupler.cpp`, `docs/04_acoplador_direcional.md`, `docs/07_apendice_A.md` | parcial | estrutura física refletida na Eq. (34), sem cobertura dimensional total |
| limite assintótico do apêndice $E^y_{pq}$ | Eq. (57), Eq. (58) | `docs/03.1_modos_Ey.md`, `docs/11_closed_form_vs_exact.md`, `src/physics/single_guide.cpp` | conceitual + implementado | base da validade de `closed_form` |
| bloco do apêndice para $E^x_{pq}$ | Eq. (59) a Eq. (64) | `src/physics/coupler.cpp`, `docs/07_apendice_A.md`, `src/physics/fig11.cpp` | parcial | base física do ramo `eq20`; cobertura ainda em forma reduzida |
| curvas de dispersão do guia | Fig. 6 | `src/physics/fig6.cpp`, `src/io/fig6_io.cpp`, `scripts/plot_fig6.py`, `data/input/fig6/*.json` | implementado | concordância boa, ainda com ajustes finos e acabamento editorial |
| nomograma de projeto | Fig. 7 | `src/physics/fig7.cpp`, `src/io/fig7_io.cpp`, `scripts/plot_fig7.py`, `data/input/reproduce_fig7.json` | implementado | boa coerência física; fac-símile visual ainda parcial |
| guia com interface metalizada | Fig. 8 | `src/physics/metal_guide.cpp`, `src/physics/fig8.cpp`, `src/io/fig8_io.cpp`, `scripts/plot_fig8.py`, `data/input/reproduce_fig8.json` | implementado com OCR aberto | física plausível, interpretação modal ainda em revisão |
| acoplamento normalizado, caso Fig. 10 | Fig. 10 | `src/physics/coupler.cpp`, `src/physics/fig10.cpp`, `src/io/fig10_io.cpp`, `scripts/plot_fig10.py`, `data/input/reproduce_fig10.json` | implementado com hipótese aberta | pendência principal: família `1.0` versus `1.6` e referência de Jones |
| acoplamento normalizado, caso Fig. 11 | Fig. 11 | `src/physics/coupler.cpp`, `src/physics/fig11.cpp`, `src/io/fig11_io.cpp`, `scripts/plot_fig11.py`, `data/input/reproduce_fig11.json` | implementado | baseline atual usa apenas solver `exact` |
| dimensões monomodo | Tabela I | `src/physics/table1.cpp`, `src/io/table1_io.cpp`, `data/input/reproduce_table1.json` | implementado | `table_entry_interpretation = a_times_n1_over_lambda` congelada na API, com `cutoff_status` explícito e regressões quantitativas |
| fluxo de casos por arquivo de entrada | entrada por JSON | `src/apps/*.cpp`, `src/io/*.cpp`, `data/input/*.json` | implementado | coerência estrutural forte |
| saída numérica rastreável | JSON e CSV | `src/io/*.cpp`, `data/output/`, `scripts/plot_fig*.py` | implementado | um dos pontos mais sólidos do projeto |
| validação automática de execução | smoke tests + regressões | `CMakeLists.txt`, `tests/regression_checks.cpp` | implementado | há `10` smoke tests e `1` suíte de regressão cobrindo figuras, Tabela I, parser legado e correções físicas |

## Leitura da matriz

O status **implementado** significa que já existe cobertura operacional no código e no fluxo de entrada/saída.

O status **parcial** significa que a peça principal existe, mas ainda com alguma destas limitações:

- cobertura apenas em forma normalizada;
- dependência de hipótese editorial ainda aberta;
- ausência de checagem quantitativa fechada.

## Conclusão rápida

A matriz mostra um padrão claro:

- o miolo do guia único está bem coberto;
- o nomograma e as figuras principais existem e são reprodutíveis;
- o acoplador está funcional em sua forma normalizada e já expõe uma camada dimensional do mesmo modelo reduzido;
- o maior débito restante está no acabamento científico e no fechamento das ambiguidades abertas, não na ausência completa de implementação.


<!-- NAV START -->
---

**Navegação:** [Anterior](21_validacao_figura_por_figura.md) | [Índice](00_resumo.md) | [Checklist](09_checklist_reproducao.md) | [Roteiro](15_roteiro_de_estudo.md) | [Riscos](23_riscos_tecnicos_e_pendencias.md) | [Próximo](23_riscos_tecnicos_e_pendencias.md)

<!-- NAV END -->
