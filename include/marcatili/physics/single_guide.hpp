#pragma once

// ============================================================================
// Tipos e funções do solver do guia dielétrico retangular único
// ============================================================================
//
// Este header define a interface pública para resolver o problema do guia
// dielétrico retangular da Seção III do artigo de Marcatili (1969).
//
// Geometria (Fig. 3 do artigo):
//
//         ┌──────┬───────────┬──────┐
//         │  n₃  │    n₄     │  n₃  │  (meio externo superior é n₄)
//         ├──────┼───────────┼──────┤
//         │  n₃  │    n₁     │  n₅  │  ←  núcleo (altura 2b, largura 2a)
//         ├──────┼───────────┼──────┤
//         │  n₃  │    n₂     │  n₃  │  (meio externo inferior é n₂)
//         └──────┴───────────┴──────┘
//              ←────── 2a ──────→
//
// Eixos: x → horizontal (direção de 'a'), y → vertical (direção de 'b')
// Propagação: eixo z (saindo da página)
//
// Convenção de numeração dos meios (Fig. 3):
//   n₁ = núcleo       n₂ = face inferior   n₃ = face esquerda
//   n₄ = face superior  n₅ = face direita
//
// Referência: Marcatili, E. A. J., Bell Syst. Tech. J. 48, 2071–2102 (1969).
// ============================================================================

#include <string>

namespace marcatili {

// ─────────────────────────────────────────────────────────────────────────────
// SingleGuideFamily
//
// As duas famílias de modos híbridos do modelo de Marcatili.
// A nomenclatura E^y e E^x indica a componente de campo elétrico dominante.
//
// Família E^y (kEy):
//   Modo predominantemente polarizado em y. Equações transversais: (6) e (7).
//   Aproximação em forma fechada: Eqs. (12) e (13).
//   Fatores de índice n²/n₁² aparecem na dimensão y (Eq. 7), não em x (Eq. 6).
//
// Família E^x (kEx):
//   Modo predominantemente polarizado em x. Equações transversais: (20) e (21).
//   Aproximação em forma fechada: Eqs. (22) e (23).
//   Fatores de índice n²/n₁² aparecem na dimensão x (Eq. 20), não em y (Eq. 21).
//   É o análogo "rotacionado" da família E^y.
// ─────────────────────────────────────────────────────────────────────────────
enum class SingleGuideFamily {
    kEy,   // família E^y: componente dominante E_y, Eqs. (6), (7), (12), (13)
    kEx    // família E^x: componente dominante E_x, Eqs. (20), (21), (22), (23)
};

// ─────────────────────────────────────────────────────────────────────────────
// SingleGuideSolverModel
//
// Seleciona entre os dois métodos de solução das equações transversais.
//
// kClosedForm:
//   Usa as fórmulas algébricas explícitas derivadas pela expansão assintótica
//   de Marcatili para modos bem guiados. É mais rápido e produz fórmulas
//   fechadas para k_x, k_y, ξ e η. Válido quando (k_t · A_v / π)² ≪ 1.
//
// kExact:
//   Resolve numericamente (bisseção) as equações transcendentais do modelo
//   reduzido separável. Aqui "exact" não significa o problema vetorial 2D
//   completo — significa apenas que a equação do modelo separável é resolvida
//   sem a aproximação adicional de forma fechada.
// ─────────────────────────────────────────────────────────────────────────────
enum class SingleGuideSolverModel {
    kClosedForm,   // solução algébrica: Eqs. (12)–(13) ou (22)–(23)
    kExact         // solução numérica: Eqs. (6)–(7) ou (20)–(21)
};

// ─────────────────────────────────────────────────────────────────────────────
// SingleGuideConfig
//
// Todos os parâmetros de entrada de um ponto de avaliação do guia único.
// Corresponde aos dados geométricos e materiais de uma linha de varredura
// nas figuras do artigo.
// ─────────────────────────────────────────────────────────────────────────────
struct SingleGuideConfig {
    // Metadados (não afetam o cálculo, apenas identificação e rastreabilidade)
    std::string case_id;          // identificador do caso (ex.: "SG-006a")
    std::string article_target;   // descrição da figura/ponto alvo no artigo
    std::string csv_output_path;  // caminho para saída CSV (se houver)

    // Seleção do algoritmo
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    SingleGuideFamily      family       = SingleGuideFamily::kEy;

    // Índices modais: p em x, q em y  (inteiros ≥ 1)
    // O modo fundamental é p=1, q=1.
    // p=2 implica um nó no campo ao longo de x (duas meias-ondas), etc.
    int p = 1;   // índice modal na direção x
    int q = 1;   // índice modal na direção y

    // Comprimento de onda no vácuo λ (metros)
    double wavelength = 0.0;

    // Geometria do núcleo (metros)
    double a = 0.0;   // semi-largura em x (eixo horizontal da Fig. 3)
    double b = 0.0;   // semi-altura em y  (eixo vertical da Fig. 3)

    // Índices de refração das cinco regiões (Fig. 3 do artigo)
    // Condição necessária: n₁ > max(n₂, n₃, n₄, n₅)
    double n1 = 0.0;   // núcleo
    double n2 = 0.0;   // face inferior
    double n3 = 0.0;   // face esquerda
    double n4 = 0.0;   // face superior
    double n5 = 0.0;   // face direita
};

// ─────────────────────────────────────────────────────────────────────────────
// SingleGuideApproximationChecks
//
// Diagnósticos da validade assintótica da solução em forma fechada.
//
// O modelo de Marcatili em forma fechada (Eqs. 12–13 e 22–23) é uma expansão
// assintótica válida quando o modo está bem acima do cutoff. A condição de
// validade (Eq. 11 e análogo para E^x) exige que cada um dos quatro termos
// abaixo seja muito menor que 1:
//
//   (k_x A₃/π)²,  (k_x A₅/π)²  →  mede proximidade do cutoff transversal em x
//   (k_y A₂/π)²,  (k_y A₄/π)²  →  mede proximidade do cutoff transversal em y
//
// Interpretação:
//   valor ≪ 1  →  modo bem guiado, aproximação confiável
//   valor → 1  →  modo próximo do cutoff, usar solver "exact"
// ─────────────────────────────────────────────────────────────────────────────
struct SingleGuideApproximationChecks {
    double kx_a3_over_pi_squared = 0.0;   // (k_x A₃/π)²
    double kx_a5_over_pi_squared = 0.0;   // (k_x A₅/π)²
    double ky_a2_over_pi_squared = 0.0;   // (k_y A₂/π)²
    double ky_a4_over_pi_squared = 0.0;   // (k_y A₄/π)²
};

// ─────────────────────────────────────────────────────────────────────────────
// SingleGuideResult
//
// Resultado completo de um ponto de avaliação do solver do guia único.
// Agrega as grandezas intermediárias e finais de um modo (p,q) para uma
// dada geometria, material e comprimento de onda.
// ─────────────────────────────────────────────────────────────────────────────
struct SingleGuideResult {
    // Cópia da configuração de entrada (para rastreabilidade completa)
    SingleGuideConfig config;

    // ── Status do resultado ──────────────────────────────────────────────
    bool domain_valid = false;   // true se os parâmetros estão no domínio do modelo
    bool guided       = false;   // true se k_z satisfaz a condição de guiamento

    std::string status;          // "ok", "below_cutoff", "outside_*_domain"
    std::string status_class;    // "solution", "physical_limit", "domain_limit"
    std::string equations_used;  // lista das equações aplicadas (ex.: "(6), (7)")

    // ── Números de onda (rad/m) ──────────────────────────────────────────
    // k₀ = 2π/λ         (vácuo)
    // kᵢ = k₀ nᵢ        (meio i)
    // Eq. implícita: todos derivados de k₀ e dos índices de refração
    double k0 = 0.0;   // número de onda no vácuo
    double k1 = 0.0;   // número de onda no núcleo       (região 1)
    double k2 = 0.0;   // número de onda na face inferior (região 2)
    double k3 = 0.0;   // número de onda na face esquerda (região 3)
    double k4 = 0.0;   // número de onda na face superior (região 4)
    double k5 = 0.0;   // número de onda na face direita  (região 5)

    // ── Parâmetros de escala A_v (metros) ───────────────────────────────
    // A_v = λ / (2 √(n₁² − nᵥ²))   →  Eq. (10) / Eq. (18)
    // A_v é a escala de comprimento do decaimento evanescente no meio v.
    double A2 = 0.0;   // escala na face inferior (meio 2)
    double A3 = 0.0;   // escala na face esquerda (meio 3)
    double A4 = 0.0;   // escala na face superior (meio 4)
    double A5 = 0.0;   // escala na face direita  (meio 5)

    // ── Constantes de propagação transversais e axial (rad/m) ───────────
    // Relação de dispersão: k_z² = k₁² − k_x² − k_y²   →  Eq. (3) / Eq. (17)
    double kx = 0.0;   // número de onda transversal em x  (raiz de Eq. 6/7/12/13/20-23)
    double ky = 0.0;   // número de onda transversal em y  (raiz de Eq. 6/7/12/13/20-23)
    double kz = 0.0;   // número de onda axial (constante de propagação)

    // ── Profundidades de penetração evanescente (metros) ────────────────
    // ξ_{3,5} = [ (π/A_{3,5})² − k_x² ]^{−½}   →  Eqs. (8)–(9) / (18)–(19)
    // η_{2,4} = [ (π/A_{2,4})² − k_y² ]^{−½}   →  Eqs. (8)–(9) / (18)–(19)
    double xi3  = 0.0;   // decaimento evanescente na face esquerda (meio 3)
    double xi5  = 0.0;   // decaimento evanescente na face direita  (meio 5)
    double eta2 = 0.0;   // decaimento evanescente na face inferior (meio 2)
    double eta4 = 0.0;   // decaimento evanescente na face superior (meio 4)

    // ── Normalizações para reprodução das figuras ────────────────────────
    // b/A₄: eixo horizontal da Fig. 6 — "largura elétrica" normalizada
    double b_over_A4 = 0.0;

    // Y = (k_z² − k₄²) / (k₁² − k₄²): eixo vertical da Fig. 6
    // Y = 0 → cutoff;  Y = 1 → modo totalmente guiado (k_z ≈ k₁)
    double kz_normalized_against_n4 = 0.0;

    // Maior índice entre n₂..n₅ e o correspondente número de onda
    // Usados para verificar a condição k_z > k_external_max (modo guiado)
    double critical_external_index        = 0.0;
    double critical_external_wave_number  = 0.0;

    // Diagnósticos da validade da aproximação de forma fechada (Eq. 11)
    SingleGuideApproximationChecks approximation_checks;
};

// ── Conversão enumeração ↔ string ────────────────────────────────────────────
std::string ToString(SingleGuideFamily family);
SingleGuideFamily ParseSingleGuideFamily(const std::string& family_text);

std::string ToString(SingleGuideSolverModel solver_model);
SingleGuideSolverModel ParseSingleGuideSolverModel(const std::string& solver_model_text);

/**
 * @brief Resolve o guia único com a aproximação algébrica em forma fechada.
 *
 * @details
 * Implementa as Eqs. (12)–(13) para a família E^y:
 *
 *           pπ/a                          qπ/b
 *   k_x = ─────────────────   k_y = ──────────────────────────────────
 *          1+(A₃+A₅)/(πa)             1+(n₂²A₂+n₄²A₄)/(πn₁²b)
 *
 * E as Eqs. (22)–(23) para a família E^x (papéis de x e y trocados).
 *
 * Após k_x e k_y, calcula k_z (Eq. 3/17) e os decaimentos ξ, η (Eqs. 14–16
 * para E^y; Eqs. 24–26 para E^x).
 *
 * Validade: boa quando (k_t·A_v/π)² ≪ 1 para todos os meios externos.
 */
SingleGuideResult SolveSingleGuideClosedForm(const SingleGuideConfig& config);

/**
 * @brief Resolve o guia único numericamente (bisseção nas equações transcendentais).
 *
 * @details
 * Implementa as Eqs. (6)–(7) para a família E^y e as Eqs. (20)–(21) para E^x.
 *
 * "Exact" aqui significa que a equação transcendental do modelo separável 1D+1D
 * é resolvida sem aproximação adicional — NÃO é a solução vetorial 2D completa.
 *
 * Cada equação é resolvida por bisseção no intervalo (ε, min(π/A_j, π/A_k)·(1−ε)).
 */
SingleGuideResult SolveSingleGuideExact(const SingleGuideConfig& config);

/**
 * @brief Dispatcher principal: seleciona o solver baseado em config.solver_model.
 */
SingleGuideResult SolveSingleGuide(const SingleGuideConfig& config);

}  // namespace marcatili
