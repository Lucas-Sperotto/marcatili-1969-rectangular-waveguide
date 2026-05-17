#pragma once

// ============================================================================
// Tipos e funções do solver do acoplador direcional normalizado
// ============================================================================
//
// Este header define a interface para resolver o modelo de acoplamento
// fraco entre dois guias dielétricos retangulares paralelos, descrito
// na Seção IV e no Apêndice A do artigo de Marcatili (1969).
//
// GEOMETRIA DO ACOPLADOR (Fig. 9 do artigo)
//
//   ┌──────────────────┐     c      ┌──────────────────┐
//   │   Guia 1 (n₁)    │◄──────────►│   Guia 2 (n₁)    │
//   │    largura 2a    │           │    largura 2a    │
//   └──────────────────┘           └──────────────────┘
//                 n₅                    n₅
//
//   a = semi-largura de cada guia (mesmos em ambos)
//   c = separação entre as faces internas dos dois guias
//   n₅ = índice do meio entre os dois guias
//
// NORMALIZAÇÃO
//
//   O artigo (Seção IV) trabalha com variáveis normalizadas para tornar os
//   gráficos independentes do comprimento de onda:
//
//     u = k_x A₅/π      (raiz transversal normalizada, u ∈ (0,1))
//     a/A₅               (largura elétrica normalizada do guia)
//     c/a                (separação normalizada pela largura)
//
//   onde A₅ = λ/(2√(n₁²−n₅²)) é a escala de decaimento no meio entre guias.
//
// SAÍDA PRINCIPAL
//
//   A saída central é o acoplamento normalizado (Eq. 34):
//
//     K_norm = 2 u² √(1−u²) exp(−π (c/A₅) √(1−u²))
//
//   que é plotado nas Figs. 10 e 11 em função de c/a, com a/A₅ como parâmetro.
//
// Referência: Marcatili, E. A. J., Bell Syst. Tech. J. 48, 2071–2102 (1969).
// ============================================================================

#include <string>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

// ─────────────────────────────────────────────────────────────────────────────
// CouplerTransverseEquation
//
// Seleciona qual equação transversal do guia único é usada como base para
// a resolução do acoplador. A diferença está na família modal assumida:
//
//   kEq6  → base na Eq. (6) da família E^y
//            Sem fatores n²/n₁² na equação transversal em x
//            Usada para reproduzir a Fig. 10
//
//   kEq20 → base na Eq. (20) da família E^x
//            Com fatores n²/n₁² = (n₅/n₁)² na equação transversal em x
//            Usada para reproduzir a Fig. 11
//
// No limite simétrico (n₃ = n₅) adotado pela Seção IV, as equações ficam:
//
//   Eq. (6) normalizada:   π(a/A₅)u + 2 atan(u/√(1−u²)) = pπ
//   Eq. (20) normalizada:  π(a/A₅)u + 2 atan(r u/√(1−u²)) = pπ
//
//   onde r = (n₅/n₁)²  e  u = k_x A₅/π.
// ─────────────────────────────────────────────────────────────────────────────
enum class CouplerTransverseEquation {
    kEq6,    // família E^y, Eq. (6) normalizada → Fig. 10
    kEq20    // família E^x, Eq. (20) normalizada → Fig. 11
};

struct CouplerGuideConfig {
    double a = 0.0;
    double b = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;
};

// ─────────────────────────────────────────────────────────────────────────────
// CouplerPointConfig
//
// Parâmetros de entrada para um único ponto de avaliação do acoplador.
//
// O solver trabalha em dois modos:
//
//   Modo normalizado (mínimo):
//     Requer apenas: a_over_A5, c_over_a, p, transverse_equation.
//     Produz apenas: kx_A5_over_pi (u) e normalized_coupling.
//
//   Modo dimensional (opcional):
//     Requer adicionalmente: wavelength, n1, n5.
//     Produz também: A5, a, c, kx, kz, coupling_magnitude, full_transfer_length.
// ─────────────────────────────────────────────────────────────────────────────
struct CouplerPointConfig {
    // Metadados (não afetam o cálculo)
    std::string case_id;
    std::string article_target;
    std::string csv_output_path;

    // ── Seleção do algoritmo ─────────────────────────────────────────────
    // Herda o enum do guia único:
    //   kClosedForm → usa a versão algébrica da equação normalizada
    //   kExact      → resolve numericamente a equação transcendental normalizada
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;

    // Seleciona a equação base (E^y ou E^x)
    CouplerTransverseEquation transverse_equation = CouplerTransverseEquation::kEq6;

    // ── Parâmetros modais ────────────────────────────────────────────────
    // Índice modal horizontal p ≥ 1
    // O modo fundamental do guia é p = 1 (meia-onda em x).
    int p = 1;

    // Índice modal vertical q ≥ 1.
    // O caminho histórico do acoplador é efetivamente 1D e usa q = 1.
    // O caso perturbado usa q ao resolver beta_1 e beta_2 com SolveSingleGuideExact.
    int q = 1;

    // ── Parâmetros geométricos normalizados ──────────────────────────────
    // a/A₅ = (2a/λ)√(n₁²−n₅²): largura elétrica normalizada do guia
    // Parâmetro de família das curvas nas Figs. 10 e 11.
    double a_over_A5 = 0.0;

    // c/a: separação entre os guias normalizada pela largura do guia
    // Eixo horizontal das Figs. 10 e 11.
    double c_over_a = 0.0;

    // ── Razão de índice ao quadrado ──────────────────────────────────────
    // r = (n₅/n₁)²: necessária para Eq. (20) normalizada e correspondente
    // forma fechada. Para Eq. (6), este campo não é usado.
    double index_ratio_squared = 0.0;

    // ── Parâmetros dimensionais opcionais ────────────────────────────────
    // Quando presentes (todos os três), permitem recuperar as grandezas
    // dimensionais: A₅, a, c, k_x, k_z, |K| e L.
    // Quando ausentes (= 0), apenas as saídas normalizadas são calculadas.
    double wavelength = 0.0;   // λ (metros)
    double n1         = 0.0;   // índice do núcleo
    double n5         = 0.0;   // índice do meio entre os guias

    // ── Extensão da Seção V: guias ligeiramente diferentes ───────────────
    // Quando guide_1 e guide_2 aparecem no JSON, beta_1 e beta_2 são
    // calculados independentemente com o solver exato do guia único. O
    // comprimento de acoplamento passa a usar
    // L_c = pi / (2 sqrt(kappa^2 + delta^2)), delta = (beta_1 - beta_2)/2.
    bool perturbed_guides_enabled = false;
    CouplerGuideConfig guide_1;
    CouplerGuideConfig guide_2;
};

// ─────────────────────────────────────────────────────────────────────────────
// CouplerPointResult
//
// Resultado de um ponto de avaliação do acoplador.
//
// As saídas são organizadas em dois grupos:
//   1. Grandezas normalizadas (sempre disponíveis quando domain_valid = true)
//   2. Grandezas dimensionais (disponíveis quando dimensional_outputs_available)
// ─────────────────────────────────────────────────────────────────────────────
struct CouplerPointResult {
    // Cópia da configuração de entrada
    CouplerPointConfig config;

    // ── Status ──────────────────────────────────────────────────────────
    bool domain_valid                 = false;
    bool transverse_root_found        = false;   // true se a raiz u foi encontrada
    bool dimensional_outputs_available = false;  // true se A₅, |K|, L foram calculados

    std::string status;          // "ok", "below_transverse_cutoff", "outside_*"
    std::string status_class;    // "solution", "physical_limit", "domain_limit"
    std::string equations_used;  // ex.: "(6), (8), (33), (34)"

    // ── Grandezas normalizadas (Seção IV) ────────────────────────────────
    // a/A₅: recopiado da configuração para conveniência de saída
    double a_over_A5 = 0.0;

    // c/A₅ = (c/a)·(a/A₅): separação normalizada por A₅
    // Este é o argumento do expoente em Eq. (34): π(c/A₅)√(1−u²)
    double c_over_A5 = 0.0;

    // u = k_x A₅/π: raiz transversal normalizada  (u ∈ (0,1) para modo guiado)
    // Obtida resolvendo Eq. (6) ou Eq. (20) normalizada.
    double kx_A5_over_pi = 0.0;

    // √(1−u²): fator evanescente normalizado
    // Aparece tanto no expoente quanto no pré-fator de Eq. (34)
    double sqrt_one_minus_kx_A5_over_pi_squared = 0.0;

    // K_norm = 2u²√(1−u²) exp(−π(c/A₅)√(1−u²))   →  Eq. (34) normalizada
    // Saída central das Figs. 10 e 11.
    double normalized_coupling = 0.0;

    // ── Grandezas dimensionais (requerem wavelength, n1, n5) ────────────
    double A5  = 0.0;   // escala de decaimento no meio entre guias  →  Eq. (10)
    double a   = 0.0;   // semi-largura dimensional do guia  (= a/A₅ · A₅)
    double c   = 0.0;   // separação dimensional              (= c/a · a)
    double k0  = 0.0;   // k₀ = 2π/λ
    double k1  = 0.0;   // k₁ = k₀ n₁
    double k5  = 0.0;   // k₅ = k₀ n₅
    double kx  = 0.0;   // k_x dimensional  (= u · π/A₅)
    double kz  = 0.0;   // k_z = √(k₁² − k_x²)  (slab: ignora k_y)

    // |K|: magnitude do coeficiente de acoplamento por unidade de comprimento
    // Derivada de K_norm via: |K| = K_norm · √(1−(n₅/n₁)²) · k_z / a
    double coupling_magnitude    = 0.0;

    // L = π/(2|K|): comprimento de transferência total
    // Distância ao longo de z para transferência completa de potência entre guias
    double full_transfer_length  = 0.0;

    // ── Extensão da Seção V: guias ligeiramente diferentes ───────────────
    bool perturbed_outputs_available = false;
    double beta_1 = 0.0;
    double beta_2 = 0.0;
    double delta = 0.0;
    double effective_coupling_magnitude = 0.0;
};

// ── Conversão enumeração ↔ string ────────────────────────────────────────────
std::string ToString(CouplerTransverseEquation equation);

/**
 * @throws std::invalid_argument Se o texto não for "eq6" ou "eq20".
 */
CouplerTransverseEquation ParseCouplerTransverseEquation(
    const std::string& equation_text
);

/**
 * @brief Resolve um ponto do modelo normalizado do acoplador.
 *
 * @details
 * O cálculo ocorre em duas etapas:
 *
 * 1. Resolver a raiz transversal normalizada u = k_x A₅/π:
 *    - Eq. (6) normalizada (kEq6):   π(a/A₅)u + 2 atan(u/√(1−u²)) = pπ
 *    - Eq. (20) normalizada (kEq20): π(a/A₅)u + 2 atan(r u/√(1−u²)) = pπ
 *
 * 2. Aplicar Eq. (34) para o acoplamento normalizado:
 *    K_norm = 2u²√(1−u²) exp(−π(c/A₅)√(1−u²))
 *    com c/A₅ = (c/a)·(a/A₅).
 *
 * @throws std::invalid_argument Se a configuração de entrada for inválida.
 * @throws std::runtime_error Se a busca da raiz falhar numericamente.
 */
CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config);

}  // namespace marcatili
