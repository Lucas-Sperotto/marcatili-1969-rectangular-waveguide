#include "marcatili/physics/coupler.hpp"

// ============================================================================
// Solver do acoplador direcional normalizado — Modelo de Marcatili (1969)
// ============================================================================
//
// Este arquivo implementa a Seção IV e parte do Apêndice A do artigo para
// o modelo de acoplamento fraco entre dois guias retangulares paralelos.
//
// CONCEITO FÍSICO
// ───────────────
// Quando dois guias dielétricos são colocados próximos, os campos evanescentes
// que se estendem para além dos núcleos se sobrepõem. Essa sobreposição cria
// um acoplamento gradual de energia entre os guias ao longo de z.
//
// O coeficiente de acoplamento |K| (rad/m) determina a taxa de transferência.
// A distância necessária para transferência completa de potência é:
//
//   L = π / (2|K|)   (comprimento de transferência total)
//
// ESTRATÉGIA DE NORMALIZAÇÃO
// ───────────────────────────
// Marcatili escreve |K| em forma normalizada que não depende explicitamente
// de λ, a, n₁, n₅ individualmente — apenas das razões adimensionais:
//
//   u = k_x A₅/π    (raiz transversal normalizada, u ∈ (0,1))
//   a/A₅            (largura elétrica do guia, parâmetro de família)
//   c/A₅            (separação normalizada por A₅)
//
// O acoplamento normalizado (Eq. 34) é:
//
//   K_norm = 2 u² √(1−u²) exp( −π (c/A₅) √(1−u²) )
//
// onde:
//   u²         → relacionado à densidade de energia transversal no núcleo
//   √(1−u²)    → fator evanescente (decaimento lateral do campo)
//   exp(...)   → fator de sobreposição dos campos evanescentes de cada guia
//
// DUAS ETAPAS DE CÁLCULO
// ──────────────────────
// Etapa 1 — Raiz transversal u:
//   A raiz u é obtida resolvendo a equação transcendental do guia único
//   reescrita em termos de u. Para o guia único, a Eq. (6) (família E^y)
//   fica, no limite simétrico n₃ = n₅:
//
//     Eq. (6) normalizada: π(a/A₅)u + 2 atan(u/√(1−u²)) = pπ
//
//   Nota: a identidade atan(u/√(1−u²)) = arcsin(u) mostra que a equação
//   é equivalente a  (a/A₅)u + (2/π) arcsin(u) = p.
//
//   Para a família E^x, Eq. (20) normalizada:
//     π(a/A₅)u + 2 atan(r·u/√(1−u²)) = pπ,   r = (n₅/n₁)²
//
//   A forma fechada correspondente (Eqs. 12 e 22 normalizadas) é:
//     Eq. (6):  u = p / (a/A₅ + 2/π)
//     Eq. (20): u = p / (a/A₅ + 2r/π)
//
// Etapa 2 — Acoplamento normalizado (Eq. 34):
//   Com u em mãos, K_norm é calculado diretamente pela fórmula acima.
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

using math::kPi;
using math::Square;

// Limites do intervalo de busca para u = k_x A₅/π ∈ (0, 1).
// A raiz física sempre está em (0, 1): u = 0 corresponde a k_x = 0
// (não há modo), u = 1 corresponde a k_x = π/A₅ (cutoff evanescente).
constexpr double kRootLowerBound = 1e-12;    // u próximo de 0, mas nunca exato
constexpr double kRootUpperBound = 1.0 - 1e-12;   // u próximo de 1, mas antes do cutoff

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

std::string ClassifyStatus(const std::string& status) {
    if (status == "ok")                      return "solution";
    if (status == "below_transverse_cutoff") return "physical_limit";
    return "domain_limit";
}

// Verifica se os três parâmetros dimensionais obrigatórios foram fornecidos.
// Quando pelo menos um está presente, todos são exigidos (consistência).
bool HasDimensionalInputs(const CouplerPointConfig& config) {
    return IsFiniteNumber(config.wavelength) && config.wavelength > 0.0 &&
           IsFiniteNumber(config.n1)         && config.n1 > 0.0 &&
           IsFiniteNumber(config.n5)         && config.n5 > 0.0;
}

// Normaliza texto para comparação insensível a maiúsculas e pontuação.
std::string NormalizeEquationText(const std::string& text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (unsigned char ch : text) {
        if (std::isalnum(ch)) {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return normalized;
}

// ─────────────────────────────────────────────────────────────────────────────
// ValidateConfig
//
// Verifica as pré-condições da configuração do acoplador:
//   • p ≥ 1  (índice modal)
//   • a/A₅ > 0  (largura elétrica positiva)
//   • c/a ≥ 0   (separação não negativa)
//   • Para Eq. (20): 0 < r < 1  (razão de índices válida)
//   • Se parâmetros dimensionais presentes: todos três juntos e n₁ > n₅
// ─────────────────────────────────────────────────────────────────────────────
void ValidateConfig(const CouplerPointConfig& config) {
    if (config.p <= 0) {
        throw std::invalid_argument("SolveCouplerPoint: p must be a positive integer.");
    }

    if (!IsFiniteNumber(config.a_over_A5) || config.a_over_A5 <= 0.0) {
        throw std::invalid_argument(
            "SolveCouplerPoint: a_over_A5 must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.c_over_a) || config.c_over_a < 0.0) {
        throw std::invalid_argument(
            "SolveCouplerPoint: c_over_a must be a finite non-negative value."
        );
    }

    // Para Eq. (20): r = (n₅/n₁)² deve estar em (0,1)
    // r < 1 é garantido por n₅ < n₁ (condição TIR)
    if (config.transverse_equation == CouplerTransverseEquation::kEq20) {
        if (!IsFiniteNumber(config.index_ratio_squared) ||
            !(config.index_ratio_squared > 0.0 && config.index_ratio_squared < 1.0)) {
            throw std::invalid_argument(
                "SolveCouplerPoint: Eq. (20) requires 0 < index_ratio_squared < 1."
            );
        }
    }

    // Parâmetros dimensionais: ou todos presentes ou nenhum
    const bool has_any_dimensional_input =
        (IsFiniteNumber(config.wavelength) && config.wavelength > 0.0) ||
        (IsFiniteNumber(config.n1) && config.n1 > 0.0) ||
        (IsFiniteNumber(config.n5) && config.n5 > 0.0);

    if (has_any_dimensional_input && !HasDimensionalInputs(config)) {
        throw std::invalid_argument(
            "SolveCouplerPoint: dimensional mode requires wavelength, n1 and n5 together."
        );
    }

    // n₁ > n₅: condição TIR no meio entre os guias
    if (HasDimensionalInputs(config) && !(config.n1 > config.n5)) {
        throw std::invalid_argument(
            "SolveCouplerPoint: dimensional mode requires n1 > n5."
        );
    }

    // Consistência entre index_ratio_squared e n₁/n₅ quando ambos fornecidos
    if (HasDimensionalInputs(config) &&
        config.transverse_equation == CouplerTransverseEquation::kEq20) {
        const double dimensional_ratio_squared = Square(config.n5 / config.n1);
        if (IsFiniteNumber(config.index_ratio_squared) &&
            config.index_ratio_squared > 0.0 &&
            std::abs(config.index_ratio_squared - dimensional_ratio_squared) > 1e-12) {
            throw std::invalid_argument(
                "SolveCouplerPoint: index_ratio_squared is inconsistent with dimensional n1/n5."
            );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SolveExactKxRatioEq6
//
// Resolve numericamente a Eq. (6) do guia único reescrita em termos da
// variável normalizada u = k_x A₅/π, no limite simétrico n₃ = n₅:
//
//   π (a/A₅) u + 2 atan( u / √(1−u²) ) = pπ
//
// onde:
//   s = a/A₅       (parâmetro de forma normalizado)
//   p              (índice modal horizontal)
//   u ∈ (0, 1)     (variável de busca)
//
// Nota: u/√(1−u²) = tan(arcsin(u)), portanto a equação é equivalente a
// escrever a condição de fase em termos do ângulo de incidência arcsin(u).
//
// Escrita como resíduo: f(u) = π s u + 2 atan(u/√(1−u²)) − pπ
// A raiz é encontrada por bisseção em u ∈ (ε, 1−ε).
// ─────────────────────────────────────────────────────────────────────────────
double SolveExactKxRatioEq6(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double p = static_cast<double>(config.p);

    const auto residual = [&](double u) {
        const double sqrt_term = std::sqrt(1.0 - Square(u));   // √(1−u²)

        // Resíduo da Eq. (6) normalizada:
        // f(u) = π(a/A₅)u + 2 atan(u/√(1−u²)) − pπ
        return kPi * s * u + 2.0 * std::atan(u / sqrt_term) - p * kPi;
    };

    return math::SolveRootByBisection(residual, kRootLowerBound, kRootUpperBound);
}

// ─────────────────────────────────────────────────────────────────────────────
// SolveExactKxRatioEq20
//
// Resolve numericamente a Eq. (20) do guia único reescrita em termos de u,
// no limite simétrico n₃ = n₅:
//
//   π (a/A₅) u + 2 atan( r · u / √(1−u²) ) = pπ
//
// onde:
//   r = (n₅/n₁)²   (razão ao quadrado dos índices de refração)
//
// DIFERENÇA em relação à Eq. (6): o argumento do atan é multiplicado por r.
// Para r < 1, os atans são menores → u é maior para o mesmo p e a/A₅.
// Fisicamente: família E^x tem ângulo de fase efetivo menor que E^y.
//
// Escrita como resíduo: f(u) = π s u + 2 atan(r u/√(1−u²)) − pπ
// ─────────────────────────────────────────────────────────────────────────────
double SolveExactKxRatioEq20(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double r = config.index_ratio_squared;   // r = (n₅/n₁)²
    const double p = static_cast<double>(config.p);

    const auto residual = [&](double u) {
        const double sqrt_term = std::sqrt(1.0 - Square(u));   // √(1−u²)

        // Resíduo da Eq. (20) normalizada:
        // f(u) = π(a/A₅)u + 2 atan(r·u/√(1−u²)) − pπ
        return kPi * s * u + 2.0 * std::atan(r * u / sqrt_term) - p * kPi;
    };

    return math::SolveRootByBisection(residual, kRootLowerBound, kRootUpperBound);
}

// ─────────────────────────────────────────────────────────────────────────────
// Status da busca da raiz transversal normalizada
//
// kFound:         raiz u encontrada com sucesso em (0, 1)
// kBelowCutoff:   f(upper) ≤ 0 → guia não suporta o modo (p) nesta geometria
// kOutsideDomain: f(lower) ≥ 0 ou NaN → configuração fora do domínio
// ─────────────────────────────────────────────────────────────────────────────
enum class RootSearchStatus {
    kFound,
    kBelowCutoff,
    kOutsideDomain
};

struct RootSearchResult {
    RootSearchStatus status = RootSearchStatus::kOutsideDomain;
    double root = NaN();
};

// Tenta resolver a equação transcendental e classifica o resultado.
template <typename ResidualFunction>
RootSearchResult SolveRootWithStatus(const ResidualFunction& residual) {
    const double f_lower = residual(kRootLowerBound);
    const double f_upper = residual(kRootUpperBound);

    // f(ε) deve ser negativo: se não for, a configuração está fora do domínio
    if (!IsFiniteNumber(f_lower) || !IsFiniteNumber(f_upper) || !(f_lower < 0.0)) {
        return {RootSearchStatus::kOutsideDomain, NaN()};
    }

    // f(1−ε) ≤ 0: equação sem mudança de sinal → guia abaixo do cutoff
    if (f_upper <= 0.0) {
        return {RootSearchStatus::kBelowCutoff, NaN()};
    }

    return {
        RootSearchStatus::kFound,
        math::SolveRootByBisection(residual, kRootLowerBound, kRootUpperBound)
    };
}

// Seleciona a equação correta e resolve com classificação de status.
RootSearchResult SolveExactKxRatioWithStatus(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double p = static_cast<double>(config.p);

    if (config.transverse_equation == CouplerTransverseEquation::kEq6) {
        // Eq. (6) normalizada: f(u) = π s u + 2 atan(u/√(1−u²)) − pπ
        const auto residual = [&](double u) {
            const double sqrt_term = std::sqrt(1.0 - Square(u));
            return kPi * s * u + 2.0 * std::atan(u / sqrt_term) - p * kPi;
        };
        return SolveRootWithStatus(residual);
    }

    if (config.transverse_equation == CouplerTransverseEquation::kEq20) {
        const double r = config.index_ratio_squared;   // r = (n₅/n₁)²
        // Eq. (20) normalizada: f(u) = π s u + 2 atan(r u/√(1−u²)) − pπ
        const auto residual = [&](double u) {
            const double sqrt_term = std::sqrt(1.0 - Square(u));
            return kPi * s * u + 2.0 * std::atan(r * u / sqrt_term) - p * kPi;
        };
        return SolveRootWithStatus(residual);
    }

    throw std::invalid_argument("SolveCouplerPoint: unsupported transverse equation.");
}

double SolveExactKxRatio(const CouplerPointConfig& config) {
    switch (config.transverse_equation) {
        case CouplerTransverseEquation::kEq6:   return SolveExactKxRatioEq6(config);
        case CouplerTransverseEquation::kEq20:  return SolveExactKxRatioEq20(config);
    }
    throw std::invalid_argument("SolveCouplerPoint: unsupported transverse equation.");
}

// Reseta todos os campos de saída que dependem da raiz u.
void ResetDependentOutputs(CouplerPointResult& result) {
    result.transverse_root_found                  = false;
    result.kx_A5_over_pi                          = NaN();
    result.sqrt_one_minus_kx_A5_over_pi_squared   = NaN();
    result.normalized_coupling                    = NaN();
    result.kx                                     = NaN();
    result.kz                                     = NaN();
    result.coupling_magnitude                     = NaN();
    result.full_transfer_length                   = NaN();
    result.dimensional_outputs_available          = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// PopulateDimensionalOutputs
//
// Converte os resultados normalizados para grandezas dimensionais quando
// wavelength, n1 e n5 estão presentes na configuração.
//
// Sequência de cálculo:
//   1. A₅ = λ/(2√(n₁²−n₅²))     →  Eq. (10)
//   2. a  = (a/A₅) · A₅
//   3. c  = (c/a)  · a
//   4. k₀ = 2π/λ,  k₁ = k₀n₁,  k₅ = k₀n₅
//   5. k_x = u · π/A₅             →  definição de u
//   6. k_z = √(k₁² − k_x²)        →  modelo de lâmina em x
//      (ignora k_y porque o acoplador é tratado como puramente transversal
//       em x na Seção IV, parágrafo após Eq. 33)
//   7. |K| = K_norm · √(1−(n₅/n₁)²) · k_z / a
//   8. L   = π/(2|K|)
// ─────────────────────────────────────────────────────────────────────────────
void PopulateDimensionalOutputs(CouplerPointResult& result) {
    if (!HasDimensionalInputs(result.config)) {
        return;   // modo normalizado: saídas dimensionais não solicitadas
    }

    // Etapas 1–4: grandezas básicas
    result.k0 = 2.0 * kPi / result.config.wavelength;                         // k₀
    result.k1 = result.k0 * result.config.n1;                                  // k₁
    result.k5 = result.k0 * result.config.n5;                                  // k₅
    result.A5 = math::ComputeA(result.config.wavelength, result.config.n1,
                               result.config.n5);                              // A₅ → Eq. (10)
    result.a  = result.config.a_over_A5 * result.A5;                          // a = (a/A₅)·A₅
    result.c  = result.config.c_over_a  * result.a;                           // c = (c/a)·a

    // Se a raiz u não foi encontrada, não há como calcular k_x e o que segue
    if (!result.transverse_root_found || !IsFiniteNumber(result.kx_A5_over_pi) ||
        !(result.A5 > 0.0)) {
        return;
    }

    // Etapa 5: k_x dimensional
    // k_x = u · π/A₅   →  desfaz a normalização u = k_x A₅/π
    result.kx = result.kx_A5_over_pi * kPi / result.A5;

    // Etapa 6: k_z (modelo de lâmina em x)
    // No acoplador, o artigo usa o limite de lâmina: k_z² = k₁² − k_x²
    // A dimensão y não entra porque o acoplamento é tratado como puramente
    // transversal em x (Seção IV, parágrafo após Eq. 33).
    const double kz_squared = Square(result.k1) - Square(result.kx);
    if (!IsFiniteNumber(kz_squared) || kz_squared <= 0.0) {
        return;
    }
    result.kz = std::sqrt(kz_squared);

    if (!IsFiniteNumber(result.normalized_coupling) || !(result.a > 0.0)) {
        return;
    }

    // Etapa 7: magnitude do coeficiente de acoplamento |K|
    // A relação entre K_norm e |K| é derivada de Eq. (33)–(34):
    //   |K| = K_norm · √(1−(n₅/n₁)²) · k_z / a
    const double refractive_factor =
        std::sqrt(1.0 - Square(result.config.n5 / result.config.n1));

    result.coupling_magnitude =
        result.normalized_coupling * refractive_factor * result.kz / result.a;

    // Etapa 8: comprimento de transferência total L = π/(2|K|)
    if (IsFiniteNumber(result.coupling_magnitude) && result.coupling_magnitude > 0.0) {
        result.full_transfer_length = kPi / (2.0 * result.coupling_magnitude);
        result.dimensional_outputs_available = true;
        return;
    }

    // Acoplamento zero (separação muito grande → underflow numérico no exp):
    // é um resultado fisicamente válido (L = ∞).
    if (IsFiniteNumber(result.coupling_magnitude) && result.coupling_magnitude == 0.0) {
        result.full_transfer_length = std::numeric_limits<double>::infinity();
        result.dimensional_outputs_available = true;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SolveClosedFormKxRatio
//
// Versão algébrica (forma fechada) da equação transversal normalizada.
//
// Para Eq. (6) no limite simétrico (Eq. 12 normalizada):
//
//   π(a/A₅)u + 2 atan(u/√(1−u²)) ≈ π(a/A₅)u + 2u  (lineariza atan para u→0)
//
//   Igualando a pπ e resolvendo para u:
//
//     u = p / ( a/A₅ + 2/π )
//
// Para Eq. (20) no limite simétrico (Eq. 22 normalizada):
//
//   π(a/A₅)u + 2 atan(r·u/√(1−u²)) ≈ π(a/A₅)u + 2r·u
//
//     u = p / ( a/A₅ + 2r/π )
//
// Validade: mesma do closed_form do guia único — bom quando u ≪ 1
// (modo bem guiado, longe do cutoff transversal).
// ─────────────────────────────────────────────────────────────────────────────
double SolveClosedFormKxRatio(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double p = static_cast<double>(config.p);

    switch (config.transverse_equation) {
        case CouplerTransverseEquation::kEq6:
            // u = p / (a/A₅ + 2/π)   →  Eq. (12) normalizada
            return p / (s + 2.0 / kPi);

        case CouplerTransverseEquation::kEq20: {
            const double r = config.index_ratio_squared;   // r = (n₅/n₁)²
            // u = p / (a/A₅ + 2r/π)   →  Eq. (22) normalizada
            return p / (s + 2.0 * r / kPi);
        }
    }

    throw std::invalid_argument("SolveCouplerPoint: unsupported transverse equation.");
}

std::string BuildEquationsUsed(const CouplerPointConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return (config.transverse_equation == CouplerTransverseEquation::kEq6)
                   ? "(6), (8), (33), (34)"
                   : "(20), (18), (33), (34)";
    }

    return (config.transverse_equation == CouplerTransverseEquation::kEq6)
               ? "(12), (34)"
               : "(22), (34)";
}

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Funções de conversão de enumeração
// ─────────────────────────────────────────────────────────────────────────────

std::string ToString(CouplerTransverseEquation equation) {
    switch (equation) {
        case CouplerTransverseEquation::kEq6:   return "eq6";
        case CouplerTransverseEquation::kEq20:  return "eq20";
    }
    return "unknown";
}

CouplerTransverseEquation ParseCouplerTransverseEquation(const std::string& equation_text) {
    const std::string normalized = NormalizeEquationText(equation_text);

    if (normalized == "eq6"  || normalized == "6")  return CouplerTransverseEquation::kEq6;
    if (normalized == "eq20" || normalized == "20") return CouplerTransverseEquation::kEq20;

    throw std::invalid_argument(
        "ParseCouplerTransverseEquation: supported values are eq6 and eq20."
    );
}

// =============================================================================
// SolveCouplerPoint
// =============================================================================
//
// Função principal do solver do acoplador. Executa as duas etapas:
//
//   Etapa 1 — Raiz transversal u = k_x A₅/π:
//     Se solver_model = exact:      bisseção na equação transcendental
//     Se solver_model = closed_form: fórmula algébrica explícita
//
//   Etapa 2 — Acoplamento normalizado (Eq. 34):
//
//     K_norm = 2 u² √(1−u²) exp( −π (c/A₅) √(1−u²) )
//
//     onde c/A₅ = (c/a) · (a/A₅)
//
// Interpretação da fórmula de acoplamento:
//   2u²           → amplitude do campo transversal no núcleo
//   √(1−u²)       → factor evanescente lateral  (= sin(arccos(u)))
//   exp(−π·c/A₅·√(1−u²))  → sobreposição dos campos evanescentes na separação c
//
// Saída principal: `normalized_coupling` (adimensional).
// Saída opcional:  grandezas dimensionais se wavelength, n1 e n5 fornecidos.
// =============================================================================
CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config) {
    ValidateConfig(config);

    CouplerPointResult result;
    result.config       = config;
    result.status       = "ok";
    result.status_class = ClassifyStatus(result.status);
    result.equations_used = BuildEquationsUsed(config);

    // Copia as normalizações para o resultado (útil para outputs CSV/JSON)
    result.a_over_A5 = config.a_over_A5;
    result.c_over_A5 = config.c_over_a * config.a_over_A5;   // c/A₅ = (c/a)·(a/A₅)

    ResetDependentOutputs(result);

    // ─────────────────────────────────────────────────────────────────────
    // Etapa 1a — Resolução da raiz transversal u
    //
    // O acoplador reutiliza a equação transcendental do guia único, reescrita
    // na variável normalizada u = k_x A₅/π. A solução "exact" aqui significa
    // resolver numericamente o modelo reduzido normalizado — não o problema
    // vetorial 2D completo de supermodos.
    // ─────────────────────────────────────────────────────────────────────
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        const RootSearchResult root = SolveExactKxRatioWithStatus(config);

        if (root.status == RootSearchStatus::kOutsideDomain) {
            result.status       = "outside_transverse_domain";
            result.status_class = ClassifyStatus(result.status);
            result.domain_valid = false;
            PopulateDimensionalOutputs(result);
            return result;
        }

        if (root.status == RootSearchStatus::kBelowCutoff) {
            result.status       = "below_transverse_cutoff";
            result.status_class = ClassifyStatus(result.status);
            result.domain_valid = true;
            PopulateDimensionalOutputs(result);
            return result;
        }

        result.kx_A5_over_pi = root.root;   // u obtido numericamente
    } else {
        // Forma fechada: u = p / (a/A₅ + 2/π) ou p / (a/A₅ + 2r/π)
        result.kx_A5_over_pi = SolveClosedFormKxRatio(config);
    }

    result.transverse_root_found = true;

    // ─────────────────────────────────────────────────────────────────────
    // Verificação de domínio: u deve estar em (0, 1)
    // u ≤ 0 ou u ≥ 1 indica problema fora do domínio físico do modelo.
    // ─────────────────────────────────────────────────────────────────────
    if (!(result.kx_A5_over_pi > 0.0 && result.kx_A5_over_pi < 1.0) ||
        !IsFiniteNumber(result.kx_A5_over_pi)) {
        result.status       = "outside_transverse_domain";
        result.status_class = ClassifyStatus(result.status);
        result.domain_valid = false;
        ResetDependentOutputs(result);
        PopulateDimensionalOutputs(result);
        return result;
    }

    // Calcula o fator evanescente normalizado: √(1−u²)
    // Este fator aparece tanto no expoente quanto no pré-fator de Eq. (34).
    const double sqrt_argument = 1.0 - Square(result.kx_A5_over_pi);   // 1 − u²
    if (!(sqrt_argument > 0.0) || !IsFiniteNumber(sqrt_argument)) {
        result.status       = "outside_transverse_domain";
        result.status_class = ClassifyStatus(result.status);
        result.domain_valid = false;
        ResetDependentOutputs(result);
        PopulateDimensionalOutputs(result);
        return result;
    }

    // √(1−u²): fator evanescente lateral normalizado
    result.sqrt_one_minus_kx_A5_over_pi_squared = std::sqrt(sqrt_argument);

    // ─────────────────────────────────────────────────────────────────────
    // Etapa 2 — Acoplamento normalizado (Eq. 34)
    //
    //   K_norm = 2 u² √(1−u²) exp( −π (c/A₅) √(1−u²) )
    //
    // onde:
    //   u                = kx_A5_over_pi
    //   √(1−u²)          = sqrt_one_minus_kx_A5_over_pi_squared
    //   c/A₅             = c_over_A5 = (c/a)·(a/A₅)
    //
    // Estrutura da fórmula:
    //   2u²         →  pré-fator da amplitude transversal do campo no núcleo
    //   √(1−u²)     →  fator evanescente lateral
    //   exp(−π(c/A₅)√(1−u²))  →  decaimento exponencial com a separação c
    //
    // Para grandes separações c, o expoente pode gerar underflow (→ 0),
    // que é um resultado físico válido (acoplamento desprezível).
    // ─────────────────────────────────────────────────────────────────────
    result.normalized_coupling =
        2.0 * Square(result.kx_A5_over_pi) *
        result.sqrt_one_minus_kx_A5_over_pi_squared *
        std::exp(
            -kPi * result.c_over_A5 * result.sqrt_one_minus_kx_A5_over_pi_squared
        );

    // Verificação de domínio: K_norm deve ser finito e não negativo
    result.domain_valid =
        IsFiniteNumber(result.normalized_coupling) &&
        result.normalized_coupling >= 0.0;

    if (!result.domain_valid) {
        result.status           = "outside_coupling_domain";
        result.status_class     = ClassifyStatus(result.status);
        result.normalized_coupling = NaN();
        PopulateDimensionalOutputs(result);
        return result;
    }

    // Calcula saídas dimensionais (opcional, se wavelength/n1/n5 presentes)
    PopulateDimensionalOutputs(result);
    return result;
}

}  // namespace marcatili
