#include "marcatili/physics/single_guide.hpp"

// ============================================================================
// Solver do guia dielétrico retangular único — Modelo de Marcatili (1969)
// ============================================================================
//
// Este arquivo implementa as Seções II e III do artigo para o guia único.
//
// A IDEIA CENTRAL DO MODELO
// ─────────────────────────
// O problema de um guia retangular com cinco regiões dielétricas distintas
// (Fig. 3) seria muito complexo de resolver exatamente. Marcatili simplifica
// em dois passos:
//
//   1. Desprezar os cantos: ignora as quatro regiões hachuradas da Fig. 3.
//      Isso permite separar o problema 2D em dois problemas 1D independentes.
//
//   2. Contraste fraco: assume n₁ ≈ nᵥ (índices próximos), o que torna os
//      modos quase-TEM e simplifica as condições de contorno.
//
// Com essas duas aproximações, encontrar o modo (p,q) equivale a:
//   • Resolver a equação transcendental em x → obtém k_x
//   • Resolver a equação transcendental em y → obtém k_y
//   • Calcular k_z = √(k₁² − k_x² − k_y²)   (Eq. 3 / Eq. 17)
//
// DUAS ESTRATÉGIAS DE SOLUÇÃO
// ───────────────────────────
//
//   SolveSingleGuideClosedForm  (solver_model = closed_form)
//   ─────────────────────────────────────────────────────────
//   Usa uma expansão assintótica adicional que produz fórmulas explícitas
//   para k_x e k_y em termos dos parâmetros geométricos e materiais.
//
//   Família E^y — Eqs. (12) e (13):
//
//              pπ/a                              qπ/b
//   k_x = ──────────────────    k_y = ─────────────────────────────────
//          1 + (A₃+A₅)/(πa)           1 + (n₂²A₂ + n₄²A₄)/(πn₁²b)
//
//   Família E^x — Eqs. (22) e (23):
//
//              pπ/a                              qπ/b
//   k_x = ───────────────────────────────    k_y = ─────────────────────
//          1 + (n₃²A₃+n₅²A₅)/(πn₁²a)              1 + (A₂+A₄)/(πb)
//
//   SolveSingleGuideExact  (solver_model = exact)
//   ───────────────────────────────────────────────
//   Resolve numericamente as equações transcendentais do modelo separável.
//   "Exact" NÃO significa o problema vetorial 2D completo — significa que
//   a equação transcendental do modelo reduzido é resolvida sem a
//   aproximação de forma fechada adicional.
//
//   Família E^y:
//     Eq. (6): k_x a + atan(k_x ξ₃) + atan(k_x ξ₅) = pπ
//     Eq. (7): k_y b + atan(n₂²/n₁² k_y η₂) + atan(n₄²/n₁² k_y η₄) = qπ
//
//   Família E^x:
//     Eq. (20): k_x a + atan(n₃²/n₁² k_x ξ₃) + atan(n₅²/n₁² k_x ξ₅) = pπ
//     Eq. (21): k_y b + atan(k_y η₂) + atan(k_y η₄) = qπ
//
// Referência: Marcatili, E. A. J., Bell Syst. Tech. J. 48, 2071–2102 (1969).
// ============================================================================

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>

#include "marcatili/math/root_finding.hpp"
#include "marcatili/math/waveguide_math.hpp"

namespace marcatili {
namespace {

using math::ComputeA;
using math::kPi;
using math::PenetrationDepth;
using math::SafeUpperBound;
using math::Square;

// Limite inferior da busca de raiz: evita k_t = 0 onde a função tem comportamento
// singular (atan(k_t·ξ) com ξ → ∞ quando k_t → 0). Um valor de 10⁻¹² é
// suficientemente próximo de zero para capturar qualquer raiz física real.
constexpr double kRootLowerBound = 1e-12;

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

// Mapeia código de status para a classe semântica usada nos relatórios.
// "solution"      → resultado fisicamente válido e modo guiado
// "physical_limit"→ resultado válido mas modo abaixo do cutoff
// "domain_limit"  → fora do domínio de validade do modelo
std::string ClassifyStatus(const std::string& status) {
    if (status == "ok")           return "solution";
    if (status == "below_cutoff") return "physical_limit";
    return "domain_limit";
}

// Normaliza texto para comparação insensível a maiúsculas e pontuação.
// Usado para interpretar strings como "E_y", "E^y", "Ey" → "ey".
std::string NormalizeToken(const std::string& text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (unsigned char ch : text) {
        if (std::isalnum(ch)) {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return normalized;
}

// Reseta todos os campos de saída que dependem de k_x e k_y.
// Chamado quando o solver não consegue produzir um resultado válido.
void ResetDependentOutputs(SingleGuideResult& result) {
    result.kz   = NaN();
    result.xi3  = NaN();
    result.xi5  = NaN();
    result.eta2 = NaN();
    result.eta4 = NaN();
    result.kz_normalized_against_n4 = NaN();
    result.guided = false;
}

void SetUnavailableResult(
    SingleGuideResult& result,
    const char* status,
    bool domain_valid
) {
    result.status       = status;
    result.status_class = ClassifyStatus(result.status);
    ResetDependentOutputs(result);
    result.domain_valid = domain_valid;
}

// ─────────────────────────────────────────────────────────────────────────────
// ValidateConfig
//
// Verifica as pré-condições físicas do modelo antes de qualquer cálculo:
//   • λ > 0, a > 0, b > 0
//   • p ≥ 1, q ≥ 1  (índices modais começam em 1)
//   • n₁, n₂, n₃, n₄, n₅ > 0
//   • n₁ > max(n₂, n₃, n₄, n₅)  ← condição de reflexão interna total (TIR)
//
// A condição TIR é estritamente necessária: sem ela, não há modo guiado
// possível pelo mecanismo assumido pelo modelo de Marcatili.
// ─────────────────────────────────────────────────────────────────────────────
void ValidateConfig(const SingleGuideConfig& config) {
    if (!IsFiniteNumber(config.wavelength) || config.wavelength <= 0.0) {
        throw std::invalid_argument(
            "SolveSingleGuide: wavelength must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.a) || !IsFiniteNumber(config.b) ||
        config.a <= 0.0 || config.b <= 0.0) {
        throw std::invalid_argument(
            "SolveSingleGuide: a and b must be finite positive values."
        );
    }

    if (config.p <= 0 || config.q <= 0) {
        throw std::invalid_argument(
            "SolveSingleGuide: mode indices p and q must be positive integers."
        );
    }

    if (!IsFiniteNumber(config.n1) || config.n1 <= 0.0 ||
        !IsFiniteNumber(config.n2) || config.n2 <= 0.0 ||
        !IsFiniteNumber(config.n3) || config.n3 <= 0.0 ||
        !IsFiniteNumber(config.n4) || config.n4 <= 0.0 ||
        !IsFiniteNumber(config.n5) || config.n5 <= 0.0) {
        throw std::invalid_argument(
            "SolveSingleGuide: refractive indices n1..n5 must be finite positive values."
        );
    }

    // Condição TIR: o núcleo precisa ter índice estritamente maior que todos
    // os meios externos para que a reflexão interna total seja possível.
    const double external_max =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));

    if (!(config.n1 > external_max)) {
        throw std::invalid_argument(
            "SolveSingleGuide: requires n1 > n2, n3, n4 and n5."
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildBaseResult
//
// Calcula todas as grandezas intermediárias comuns a ambos os solvers:
//
//   k₀ = 2π/λ                      →  número de onda no vácuo
//   kᵢ = k₀ nᵢ   (i = 1..5)       →  números de onda em cada região
//   A_v = λ/(2√(n₁²−nᵥ²))         →  Eq. (10) / Eq. (18)
//   b/A₄                           →  eixo horizontal da Fig. 6
//
// Esta função é chamada no início de ambos os solvers para evitar duplicação.
// ─────────────────────────────────────────────────────────────────────────────
SingleGuideResult BuildBaseResult(const SingleGuideConfig& config) {
    SingleGuideResult result;
    result.config       = config;
    result.status       = "ok";
    result.status_class = ClassifyStatus(result.status);

    // ── Números de onda ────────────────────────────────────────────────
    result.k0 = 2.0 * kPi / config.wavelength;   // k₀ = 2π/λ

    result.k1 = result.k0 * config.n1;   // k₁ = k₀ n₁  (núcleo)
    result.k2 = result.k0 * config.n2;   // k₂ = k₀ n₂  (face inferior)
    result.k3 = result.k0 * config.n3;   // k₃ = k₀ n₃  (face esquerda)
    result.k4 = result.k0 * config.n4;   // k₄ = k₀ n₄  (face superior)
    result.k5 = result.k0 * config.n5;   // k₅ = k₀ n₅  (face direita)

    // ── Parâmetros de escala A_v (Eq. 10 / Eq. 18) ────────────────────
    // Cada A_v governa o decaimento evanescente no meio externo v.
    result.A2 = ComputeA(config.wavelength, config.n1, config.n2);   // face inferior
    result.A3 = ComputeA(config.wavelength, config.n1, config.n3);   // face esquerda
    result.A4 = ComputeA(config.wavelength, config.n1, config.n4);   // face superior
    result.A5 = ComputeA(config.wavelength, config.n1, config.n5);   // face direita

    // ── Normalização para a Fig. 6 ────────────────────────────────────
    // b/A₄ = (2b/λ)√(n₁²−n₄²): eixo horizontal da Fig. 6
    // Representa a "largura elétrica" da dimensão vertical do guia.
    result.b_over_A4 = config.b / result.A4;

    // Maior índice externo e correspondente número de onda:
    // usados para verificar a condição de guiamento k_z > k_external_max
    result.critical_external_index =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));
    result.critical_external_wave_number = result.k0 * result.critical_external_index;

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// FinalizeResult
//
// Calcula o eixo vertical da Fig. 6 e classifica o modo como guiado ou não.
//
// Normalização do eixo Y da Fig. 6:
//
//   Y = (k_z² − k₄²) / (k₁² − k₄²)
//
// Interpretação:
//   Y = 0  → modo no cutoff (k_z = k₄, campo mal confinado)
//   Y = 1  → modo totalmente guiado (k_z ≈ k₁, campo completamente confinado)
//
// A normalização usa n₄ como referência porque as figuras do artigo adotam
// o meio 4 como denominador. Isso é uma convenção de apresentação.
//
// Condição de guiamento:
//   k_external_max < k_z ≤ k₁
//   equivalente a:  0 ≤ Y ≤ 1
// ─────────────────────────────────────────────────────────────────────────────
void FinalizeResult(SingleGuideResult& result) {
    const double denominator = Square(result.k1) - Square(result.k4);   // k₁² − k₄²

    // Y = (k_z² − k₄²) / (k₁² − k₄²)   →  eixo vertical da Fig. 6
    result.kz_normalized_against_n4 =
        (denominator > 0.0 && IsFiniteNumber(result.kz))
            ? (Square(result.kz) - Square(result.k4)) / denominator
            : NaN();

    // Modo guiado: k_z estritamente maior que todos os números de onda externos
    // e menor ou igual a k₁, com normalização Y consistente em [0,1].
    result.guided =
        IsFiniteNumber(result.kz) &&
        Square(result.kz) > Square(result.critical_external_wave_number) &&
        Square(result.kz) <= Square(result.k1) &&
        IsFiniteNumber(result.kz_normalized_against_n4) &&
        result.kz_normalized_against_n4 >= 0.0 &&
        result.kz_normalized_against_n4 <= 1.0;

    result.domain_valid = true;

    if (!result.guided) {
        result.status = "below_cutoff";
    }

    result.status_class = ClassifyStatus(result.status);
}

// Verifica a condição de Bolzano: f(lower) e f(upper) têm sinais opostos.
// Necessária para garantir que o método da bisseção encontre uma raiz.
bool BracketsRoot(
    const std::function<double(double)>& function,
    double lower,
    double upper
) {
    const double f_lower = function(lower);
    const double f_upper = function(upper);

    return IsFiniteNumber(f_lower) &&
           IsFiniteNumber(f_upper) &&
           f_lower <= 0.0 &&
           f_upper >= 0.0;
}

double SolveExactRoot(
    const std::function<double(double)>& function,
    double upper_bound
) {
    if (!BracketsRoot(function, kRootLowerBound, upper_bound)) {
        throw std::runtime_error(
            "SolveSingleGuideExact: no guided root found in the allowed transverse interval."
        );
    }

    return math::SolveRootByBisection(function, kRootLowerBound, upper_bound);
}

// ─────────────────────────────────────────────────────────────────────────────
// Classificação do resultado da busca de raiz transversal
//
// kFound:         f(ε) < 0 e f(upper) > 0  →  mudança de sinal, raiz isolada
// kBelowCutoff:   f(ε) < 0 e f(upper) ≤ 0  →  modo não guiado para este (p,q)
// kOutsideDomain: f(ε) ≥ 0 ou NaN          →  configuração fora do domínio
//
// Para as equações de Marcatili, o comportamento esperado é:
//   f(ε→0) ≈ ε·d − pπ < 0  (pois pπ >> ε·d para qualquer dimensão d realista)
//   f(upper) > 0            (a soma de atans satura ao se aproximar de π/A_v)
// ─────────────────────────────────────────────────────────────────────────────
enum class RootSearchStatus {
    kFound,          // raiz encontrada
    kBelowCutoff,    // modo não guiado (equação sem mudança de sinal)
    kOutsideDomain   // fora do domínio (função não finita ou f(lower) ≥ 0)
};

struct RootSearchResult {
    RootSearchStatus status = RootSearchStatus::kOutsideDomain;
    double root = NaN();
};

RootSearchResult SolveExactRootWithStatus(
    const std::function<double(double)>& function,
    double upper_bound
) {
    const double f_lower = function(kRootLowerBound);
    const double f_upper = function(upper_bound);

    // f(lower) deve ser negativo: condição necessária para modo físico
    if (!IsFiniteNumber(f_lower) || !IsFiniteNumber(f_upper) || !(f_lower < 0.0)) {
        return {RootSearchStatus::kOutsideDomain, NaN()};
    }

    // f(upper) ≤ 0: equação não muda de sinal → modo (p,q) abaixo do cutoff
    if (f_upper <= 0.0) {
        return {RootSearchStatus::kBelowCutoff, NaN()};
    }

    if (!BracketsRoot(function, kRootLowerBound, upper_bound)) {
        return {RootSearchStatus::kOutsideDomain, NaN()};
    }

    return {
        RootSearchStatus::kFound,
        SolveExactRoot(function, upper_bound)
    };
}

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Funções de conversão de enumeração
// ─────────────────────────────────────────────────────────────────────────────

std::string ToString(SingleGuideFamily family) {
    switch (family) {
        case SingleGuideFamily::kEy: return "E_y";
        case SingleGuideFamily::kEx: return "E_x";
    }
    return "unknown";
}

std::string ToString(SingleGuideSolverModel solver_model) {
    switch (solver_model) {
        case SingleGuideSolverModel::kClosedForm: return "closed_form";
        case SingleGuideSolverModel::kExact:      return "exact";
    }
    return "unknown";
}

SingleGuideFamily ParseSingleGuideFamily(const std::string& family_text) {
    const std::string normalized = NormalizeToken(family_text);

    if (normalized == "ey") return SingleGuideFamily::kEy;
    if (normalized == "ex") return SingleGuideFamily::kEx;

    throw std::invalid_argument(
        "ParseSingleGuideFamily: supported values are E_y, Ey, E^y, E_x, Ex and E^x."
    );
}

SingleGuideSolverModel ParseSingleGuideSolverModel(const std::string& solver_model_text) {
    const std::string normalized = NormalizeToken(solver_model_text);

    if (normalized == "closedform" || normalized == "approx" || normalized == "approximate") {
        return SingleGuideSolverModel::kClosedForm;
    }

    if (normalized == "exact" || normalized == "transcendental") {
        return SingleGuideSolverModel::kExact;
    }

    throw std::invalid_argument(
        "ParseSingleGuideSolverModel: supported values are closed_form and exact."
    );
}

// =============================================================================
// SolveSingleGuideClosedForm
// =============================================================================
//
// Implementa a solução algébrica aproximada das Seções II e III do artigo.
//
// A IDEIA DA FORMA FECHADA
// ─────────────────────────
// As equações transcendentais (6) e (7) são implícitas em k_x e k_y.
// Para modos bem guiados (onde (k_t·A_v/π)² ≪ 1), Marcatili mostra que
// a solução pode ser aproximada por fórmulas explícitas (fechadas):
//
//   As equações transcendentais têm a forma:
//     k_t·d + atan(...) + atan(...) = pπ
//
//   Para (k_t·A_v/π)² ≪ 1, cada atan pode ser linearizado:
//     atan(x) ≈ x  quando x → 0
//
//   Isso produz fórmulas explícitas para k_x e k_y.
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │ Família E^y — Eqs. (12) e (13):                                         │
// │                                                                         │
// │         pπ/a                                                            │
// │  k_x = ─────────────────                     Eq. (12)                  │
// │         1 + (A₃+A₅)/(πa)                                               │
// │                                                                         │
// │         qπ/b                                                            │
// │  k_y = ─────────────────────────────────     Eq. (13)                  │
// │         1 + (n₂²A₂ + n₄²A₄)/(πn₁²b)                                   │
// │                                                                         │
// │  Após k_x e k_y (Eqs. 14–16):                                          │
// │    k_z  = √(k₁² − k_x² − k_y²)              Eq. (14)                  │
// │    ξ_{3,5} = (A_{3,5}/π) / √(1−(k_x A_{3,5}/π)²)   Eq. (15)          │
// │    η_{2,4} = (A_{2,4}/π) / √(1−(k_y A_{2,4}/π)²)   Eq. (16)          │
// └─────────────────────────────────────────────────────────────────────────┘
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │ Família E^x — Eqs. (22) e (23):                                         │
// │                                                                         │
// │         pπ/a                                                            │
// │  k_x = ─────────────────────────────────     Eq. (22)                  │
// │         1 + (n₃²A₃ + n₅²A₅)/(πn₁²a)                                   │
// │                                                                         │
// │         qπ/b                                                            │
// │  k_y = ─────────────────                     Eq. (23)                  │
// │         1 + (A₂+A₄)/(πb)                                               │
// │                                                                         │
// │  (ξ, η e k_z análogos, com Eqs. 24–26)                                 │
// └─────────────────────────────────────────────────────────────────────────┘
//
// Diferença entre E^y e E^x:
//   - Em E^y: os fatores n²/n₁² aparecem em k_y (dimensão y), não em k_x.
//   - Em E^x: os fatores n²/n₁² aparecem em k_x (dimensão x), não em k_y.
//   Esta troca de papéis reflete a diferença de polarização entre as famílias.
// =============================================================================
SingleGuideResult SolveSingleGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    if (config.family == SingleGuideFamily::kEy) {
        // ─────────────────────────────────────────────────────────────────
        // Família E^y — Eqs. (12) e (13)
        //
        // O denominador de k_x mede a "extensão efetiva" do núcleo em x
        // causada pela penetração evanescente nos meios 3 e 5:
        //   denominador_x = 1 + (A₃ + A₅)/(πa)  >  1
        //   → k_x < pπ/a  (como num guia metálico com dimensão efetiva > a)
        //
        // O denominador de k_y inclui os fatores de índice n²/n₁² porque
        // a família E^y tem polarização elétrica predominante em y, e as
        // condições de contorno em y envolvem a descontinuidade de E_y
        // (componente tangencial diferente em cada interface horizontal).
        // ─────────────────────────────────────────────────────────────────

        // Denominador de k_x: 1 + (A₃ + A₅)/(πa)
        const double x_denominator =
            1.0 + (result.A3 + result.A5) / (kPi * config.a);

        // Denominador de k_y: 1 + (n₂²A₂ + n₄²A₄)/(πn₁²b)
        const double y_denominator =
            1.0 +
            (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
                (kPi * Square(config.n1) * config.b);

        // k_x = pπ/a / denominador_x   →   Eq. (12)
        result.kx = (static_cast<double>(config.p) * kPi / config.a) / x_denominator;

        // k_y = qπ/b / denominador_y   →   Eq. (13)
        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;

        result.equations_used = "(10), (12), (13), (14), (15), (16)";

    } else {
        // ─────────────────────────────────────────────────────────────────
        // Família E^x — Eqs. (22) e (23)
        //
        // Aqui os fatores n²/n₁² migram para o denominador de k_x,
        // porque E^x tem polarização predominante em x e as condições de
        // contorno em x envolvem a descontinuidade de E_x:
        //   denominador_x = 1 + (n₃²A₃ + n₅²A₅)/(πn₁²a)
        //
        // O denominador de k_y volta à forma simples (sem fatores de índice):
        //   denominador_y = 1 + (A₂ + A₄)/(πb)
        // ─────────────────────────────────────────────────────────────────

        // Denominador de k_x: 1 + (n₃²A₃ + n₅²A₅)/(πn₁²a)
        const double x_denominator =
            1.0 +
            (Square(config.n3) * result.A3 + Square(config.n5) * result.A5) /
                (kPi * Square(config.n1) * config.a);

        // Denominador de k_y: 1 + (A₂ + A₄)/(πb)
        const double y_denominator =
            1.0 + (result.A2 + result.A4) / (kPi * config.b);

        // k_x = pπ/a / denominador_x   →   Eq. (22)
        result.kx = (static_cast<double>(config.p) * kPi / config.a) / x_denominator;

        // k_y = qπ/b / denominador_y   →   Eq. (23)
        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;

        result.equations_used = "(10), (22), (23), (24), (25), (26)";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Diagnósticos de validade da aproximação — Eq. (11) e análogo para E^x
    //
    // O modelo de forma fechada assume que cada um dos termos abaixo é ≪ 1:
    //   (k_x A₃/π)²,  (k_x A₅/π)²   →  modo bem guiado em x
    //   (k_y A₂/π)²,  (k_y A₄/π)²   →  modo bem guiado em y
    //
    // Se algum desses termos for próximo de 1, o modo está próximo do cutoff
    // e a aproximação de forma fechada perde precisão. Nesse caso, use o solver
    // "exact" para uma solução mais precisa.
    //
    // Os valores são armazenados em `approximation_checks` para inspeção.
    // ─────────────────────────────────────────────────────────────────────────
    result.approximation_checks.kx_a3_over_pi_squared =
        Square(result.kx * result.A3 / kPi);   // (k_x A₃/π)²
    result.approximation_checks.kx_a5_over_pi_squared =
        Square(result.kx * result.A5 / kPi);   // (k_x A₅/π)²
    result.approximation_checks.ky_a2_over_pi_squared =
        Square(result.ky * result.A2 / kPi);   // (k_y A₂/π)²
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);   // (k_y A₄/π)²

    // k_z² = k₁² − k_x² − k_y²   →   Eq. (3)
    const double kz_squared =
        Square(result.k1) - Square(result.kx) - Square(result.ky);

    // ─────────────────────────────────────────────────────────────────────────
    // Profundidades de penetração na forma fechada
    //
    // Para E^y — Eqs. (15) e (16):
    //   ξ_{3,5} = (A_{3,5}/π) / √( 1 − (k_x A_{3,5}/π)² )
    //   η_{2,4} = (A_{2,4}/π) / √( 1 − (k_y A_{2,4}/π)² )
    //
    // Para E^x — Eqs. (25) e (26): mesma estrutura.
    //
    // O radicando 1 − (k_t A_v/π)² deve ser estritamente positivo.
    // Se for zero ou negativo, o modo está no cutoff ou abaixo dele.
    // ─────────────────────────────────────────────────────────────────────────

    // Radicandos: 1 − (k_t A_v/π)²
    const double xi3_argument  = 1.0 - result.approximation_checks.kx_a3_over_pi_squared;
    const double xi5_argument  = 1.0 - result.approximation_checks.kx_a5_over_pi_squared;
    const double eta2_argument = 1.0 - result.approximation_checks.ky_a2_over_pi_squared;
    const double eta4_argument = 1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    // Verificação de domínio: todos os radicandos devem ser positivos
    const bool domain_valid =
        IsFiniteNumber(kz_squared) && kz_squared >= 0.0 &&
        IsFiniteNumber(xi3_argument)  && xi3_argument  > 0.0 &&
        IsFiniteNumber(xi5_argument)  && xi5_argument  > 0.0 &&
        IsFiniteNumber(eta2_argument) && eta2_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!domain_valid) {
        SetUnavailableResult(result, "outside_closed_form_domain", false);
        return result;
    }

    // k_z = √(k₁² − k_x² − k_y²)   →   Eq. (3) / Eq. (14)
    result.kz = std::sqrt(kz_squared);

    // ξ₃ = (A₃/π) / √(1 − (k_x A₃/π)²)   →   Eq. (15)
    result.xi3  = (result.A3 / kPi) / std::sqrt(xi3_argument);
    // ξ₅ = (A₅/π) / √(1 − (k_x A₅/π)²)   →   Eq. (15)
    result.xi5  = (result.A5 / kPi) / std::sqrt(xi5_argument);
    // η₂ = (A₂/π) / √(1 − (k_y A₂/π)²)   →   Eq. (16)
    result.eta2 = (result.A2 / kPi) / std::sqrt(eta2_argument);
    // η₄ = (A₄/π) / √(1 − (k_y A₄/π)²)   →   Eq. (16)
    result.eta4 = (result.A4 / kPi) / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

// =============================================================================
// SolveSingleGuideExact
// =============================================================================
//
// Resolve numericamente as equações transcendentais do modelo separável.
//
// "Exact" NÃO é a solução vetorial 2D completa do problema de autovalor.
// É a solução numérica das equações transcendentais 1D+1D do modelo reduzido
// de Marcatili — sem a aproximação adicional que produz a forma fechada.
//
// ALGORITMO
// ─────────
//   1. Para k_x: construir o resíduo f_x(k_x) da Eq. (6) ou (20).
//      Buscar a raiz de f_x = 0 no intervalo (ε, min(π/A₃, π/A₅)·(1−ε))
//      por bisseção. O limite superior vem do cutoff transversal em x.
//
//   2. Para k_y: construir o resíduo f_y(k_y) da Eq. (7) ou (21).
//      Buscar a raiz de f_y = 0 no intervalo (ε, min(π/A₂, π/A₄)·(1−ε)).
//
//   3. Com k_x e k_y em mãos, calcular:
//        k_z = √(k₁² − k_x² − k_y²)         →  Eq. (3) / Eq. (17)
//        ξ_{3,5} = 1/√((π/A_{3,5})² − k_x²) →  Eqs. (8)–(9)
//        η_{2,4} = 1/√((π/A_{2,4})² − k_y²) →  Eqs. (8)–(9)
//
// DIFERENÇA ENTRE AS FAMÍLIAS
// ───────────────────────────
//   Família E^y:
//     f_x usa Eq. (6): sem fatores n²/n₁² em x
//     f_y usa Eq. (7): com fatores n²/n₁² em y
//
//   Família E^x:
//     f_x usa Eq. (20): com fatores n²/n₁² em x
//     f_y usa Eq. (21): sem fatores n²/n₁² em y
//
//   Esta simetria especular entre as famílias é uma característica fundamental
//   do modelo de Marcatili.
// =============================================================================
SingleGuideResult SolveSingleGuideExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    RootSearchResult root_x;
    RootSearchResult root_y;

    if (config.family == SingleGuideFamily::kEy) {
        // ─────────────────────────────────────────────────────────────────
        // Família E^y
        //
        // Eq. (6) — equação transcendental em k_x:
        //
        //   k_x · a + atan(k_x · ξ₃) + atan(k_x · ξ₅) = pπ
        //
        // onde:
        //   ξ₃ = [ (π/A₃)² − k_x² ]^{−½}   →  Eq. (8): decaimento no meio 3
        //   ξ₅ = [ (π/A₅)² − k_x² ]^{−½}   →  Eq. (9): decaimento no meio 5
        //
        // Interpretação: a equação é a condição de fase de uma onda plana
        // que percorre o núcleo em x, reflete nas paredes com fase acumulada
        // atan(k_x·ξ) em cada interface, e retorna ao ponto de partida com
        // fase total pπ (p meias-ondas no núcleo de largura 2a).
        //
        // Escrita como resíduo: f_x(k_x) = LHS − pπ,  buscamos f_x = 0.
        // ─────────────────────────────────────────────────────────────────
        const auto fx = [&](double kx_value) {
            // Profundidades de penetração para o k_x tentativo
            const double xi3 = PenetrationDepth(result.A3, kx_value);   // Eq. (8)
            const double xi5 = PenetrationDepth(result.A5, kx_value);   // Eq. (9)

            // Resíduo de Eq. (6): f(k_x) = k_x a + atan(k_x ξ₃) + atan(k_x ξ₅) − pπ
            return kx_value * config.a
                 + std::atan(kx_value * xi3)
                 + std::atan(kx_value * xi5)
                 - static_cast<double>(config.p) * kPi;
        };

        // ─────────────────────────────────────────────────────────────────
        // Eq. (7) — equação transcendental em k_y:
        //
        //   k_y · b + atan(n₂²/n₁² · k_y · η₂) + atan(n₄²/n₁² · k_y · η₄) = qπ
        //
        // onde:
        //   η₂ = [ (π/A₂)² − k_y² ]^{−½}   →  Eq. (8): decaimento no meio 2
        //   η₄ = [ (π/A₄)² − k_y² ]^{−½}   →  Eq. (9): decaimento no meio 4
        //
        // A diferença em relação à Eq. (6) está nos fatores n²/n₁² dentro dos
        // atans. Esses fatores aparecem porque a família E^y tem polarização
        // dominante em y, e as condições de contorno para E_y na interface
        // horizontal introduzem a razão dos quadrados dos índices.
        //
        // Escrita como resíduo: f_y(k_y) = LHS − qπ,  buscamos f_y = 0.
        // ─────────────────────────────────────────────────────────────────
        const auto fy = [&](double ky_value) {
            const double eta2 = PenetrationDepth(result.A2, ky_value);   // Eq. (8)
            const double eta4 = PenetrationDepth(result.A4, ky_value);   // Eq. (9)

            // Resíduo de Eq. (7):
            // f(k_y) = k_y b + atan(n₂²/n₁² · k_y η₂) + atan(n₄²/n₁² · k_y η₄) − qπ
            return ky_value * config.b
                 + std::atan((Square(config.n2) / Square(config.n1)) * ky_value * eta2)
                 + std::atan((Square(config.n4) / Square(config.n1)) * ky_value * eta4)
                 - static_cast<double>(config.q) * kPi;
        };

        // Intervalo de busca para k_x: (ε,  min(π/A₃, π/A₅)·(1−ε))
        // Limite superior: cutoff transversal em x (menor dos dois meios laterais)
        root_x = SolveExactRootWithStatus(
            fx,
            SafeUpperBound(kPi / result.A3, kPi / result.A5)
        );

        // Intervalo de busca para k_y: (ε,  min(π/A₂, π/A₄)·(1−ε))
        root_y = SolveExactRootWithStatus(
            fy,
            SafeUpperBound(kPi / result.A2, kPi / result.A4)
        );

        result.equations_used = "(6), (7), (8), (9), (10)";

    } else {
        // ─────────────────────────────────────────────────────────────────
        // Família E^x
        //
        // Eq. (20) — equação transcendental em k_x:
        //
        //   k_x · a + atan(n₃²/n₁² · k_x · ξ₃) + atan(n₅²/n₁² · k_x · ξ₅) = pπ
        //
        // DIFERENÇA em relação à Eq. (6) da família E^y:
        //   - Aqui os fatores n²/n₁² aparecem em x (não em y).
        //   - Isso porque E^x tem polarização dominante em x, e as condições
        //     de contorno para E_x na interface vertical introduzem n²/n₁².
        //
        // Escrita como resíduo: f_x(k_x) = LHS − pπ,  buscamos f_x = 0.
        // ─────────────────────────────────────────────────────────────────
        const auto fx = [&](double kx_value) {
            const double xi3 = PenetrationDepth(result.A3, kx_value);   // Eq. (18)
            const double xi5 = PenetrationDepth(result.A5, kx_value);   // Eq. (19)

            // Resíduo de Eq. (20):
            // f(k_x) = k_x a + atan(n₃²/n₁² · k_x ξ₃) + atan(n₅²/n₁² · k_x ξ₅) − pπ
            return kx_value * config.a
                 + std::atan((Square(config.n3) / Square(config.n1)) * kx_value * xi3)
                 + std::atan((Square(config.n5) / Square(config.n1)) * kx_value * xi5)
                 - static_cast<double>(config.p) * kPi;
        };

        // ─────────────────────────────────────────────────────────────────
        // Eq. (21) — equação transcendental em k_y:
        //
        //   k_y · b + atan(k_y · η₂) + atan(k_y · η₄) = qπ
        //
        // Forma idêntica à Eq. (6) da família E^y (sem fatores n²/n₁²),
        // confirmando a simetria especular entre as duas famílias:
        //   E^y tem n²/n₁² em y  (Eq. 7)  →  E^x tem n²/n₁² em x  (Eq. 20)
        //   E^y sem n²/n₁² em x  (Eq. 6)  →  E^x sem n²/n₁² em y  (Eq. 21)
        //
        // Escrita como resíduo: f_y(k_y) = LHS − qπ,  buscamos f_y = 0.
        // ─────────────────────────────────────────────────────────────────
        const auto fy = [&](double ky_value) {
            const double eta2 = PenetrationDepth(result.A2, ky_value);   // Eq. (18)
            const double eta4 = PenetrationDepth(result.A4, ky_value);   // Eq. (19)

            // Resíduo de Eq. (21): f(k_y) = k_y b + atan(k_y η₂) + atan(k_y η₄) − qπ
            return ky_value * config.b
                 + std::atan(ky_value * eta2)
                 + std::atan(ky_value * eta4)
                 - static_cast<double>(config.q) * kPi;
        };

        root_x = SolveExactRootWithStatus(
            fx,
            SafeUpperBound(kPi / result.A3, kPi / result.A5)
        );
        root_y = SolveExactRootWithStatus(
            fy,
            SafeUpperBound(kPi / result.A2, kPi / result.A4)
        );

        result.equations_used = "(20), (21), (18), (19), (10)";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Diagnóstico do resultado da busca de raiz
    // ─────────────────────────────────────────────────────────────────────────

    // kOutsideDomain: f(ε) ≥ 0 ou NaN — configuração não pertence ao modelo
    if (root_x.status == RootSearchStatus::kOutsideDomain ||
        root_y.status == RootSearchStatus::kOutsideDomain) {
        result.kx = NaN();
        result.ky = NaN();
        SetUnavailableResult(result, "outside_exact_domain", false);
        return result;
    }

    // kBelowCutoff: f(upper) ≤ 0 — modo (p,q) não guiado nesta geometria
    if (root_x.status == RootSearchStatus::kBelowCutoff ||
        root_y.status == RootSearchStatus::kBelowCutoff) {
        result.kx = NaN();
        result.ky = NaN();
        SetUnavailableResult(result, "below_cutoff", true);
        return result;
    }

    // Raiz encontrada em ambas as direções
    result.kx = root_x.root;
    result.ky = root_y.root;

    // Diagnósticos de validade da aproximação (mesmo critério do closed_form)
    result.approximation_checks.kx_a3_over_pi_squared =
        Square(result.kx * result.A3 / kPi);   // (k_x A₃/π)²
    result.approximation_checks.kx_a5_over_pi_squared =
        Square(result.kx * result.A5 / kPi);   // (k_x A₅/π)²
    result.approximation_checks.ky_a2_over_pi_squared =
        Square(result.ky * result.A2 / kPi);   // (k_y A₂/π)²
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);   // (k_y A₄/π)²

    // ─────────────────────────────────────────────────────────────────────────
    // Reconstrução de k_z e dos decaimentos evanescentes a partir das raízes
    //
    //   k_z = √(k₁² − k_x² − k_y²)              →  Eq. (3) / Eq. (17)
    //   ξ₃ = 1/√((π/A₃)² − k_x²)               →  Eq. (8)
    //   ξ₅ = 1/√((π/A₅)² − k_x²)               →  Eq. (9)
    //   η₂ = 1/√((π/A₂)² − k_y²)               →  Eq. (8)
    //   η₄ = 1/√((π/A₄)² − k_y²)               →  Eq. (9)
    //
    // No solver exact, os decaimentos são calculados diretamente dos radicandos
    // sem a aproximação simplificada usada no closed_form. Isso produz valores
    // ligeiramente mais precisos quando o modo está próximo do cutoff.
    // ─────────────────────────────────────────────────────────────────────────
    const double kz_squared =
        Square(result.k1) - Square(result.kx) - Square(result.ky);   // k₁² − k_x² − k_y²

    // Radicandos dos decaimentos: (π/A_v)² − k_t²
    const double xi3_argument  = Square(kPi / result.A3) - Square(result.kx);
    const double xi5_argument  = Square(kPi / result.A5) - Square(result.kx);
    const double eta2_argument = Square(kPi / result.A2) - Square(result.ky);
    const double eta4_argument = Square(kPi / result.A4) - Square(result.ky);

    const bool decay_domain_valid =
        IsFiniteNumber(xi3_argument)  && xi3_argument  > 0.0 &&
        IsFiniteNumber(xi5_argument)  && xi5_argument  > 0.0 &&
        IsFiniteNumber(eta2_argument) && eta2_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!IsFiniteNumber(kz_squared) || !decay_domain_valid) {
        SetUnavailableResult(result, "outside_exact_domain", false);
        return result;
    }

    if (kz_squared < 0.0) {
        SetUnavailableResult(result, "below_cutoff", true);
        return result;
    }

    // k_z = √(k₁² − k_x² − k_y²)   →   Eq. (3) / Eq. (17)
    result.kz = std::sqrt(kz_squared);

    // ξ₃ = 1/√((π/A₃)² − k_x²),  ξ₅ = 1/√((π/A₅)² − k_x²)   →  Eqs. (8)–(9)
    result.xi3  = 1.0 / std::sqrt(xi3_argument);
    result.xi5  = 1.0 / std::sqrt(xi5_argument);

    // η₂ = 1/√((π/A₂)² − k_y²),  η₄ = 1/√((π/A₄)² − k_y²)   →  Eqs. (8)–(9)
    result.eta2 = 1.0 / std::sqrt(eta2_argument);
    result.eta4 = 1.0 / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

// =============================================================================
// SolveSingleGuide — dispatcher principal
//
// Seleciona automaticamente o solver conforme config.solver_model e delega.
// =============================================================================
SingleGuideResult SolveSingleGuide(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveSingleGuideExact(config);
    }

    return SolveSingleGuideClosedForm(config);
}

}  // namespace marcatili
