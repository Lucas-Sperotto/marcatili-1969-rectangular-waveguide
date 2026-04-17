#include "marcatili/physics/coupler.hpp"

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

constexpr double kRootLowerBound = 1e-12;
constexpr double kRootUpperBound = 1.0 - 1e-12;

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

std::string ClassifyStatus(const std::string& status) {
    if (status == "ok") {
        return "solution";
    }

    if (status == "below_transverse_cutoff") {
        return "physical_limit";
    }

    return "domain_limit";
}

bool HasDimensionalInputs(const CouplerPointConfig& config) {
    return IsFiniteNumber(config.wavelength) && config.wavelength > 0.0 &&
           IsFiniteNumber(config.n1) && config.n1 > 0.0 &&
           IsFiniteNumber(config.n5) && config.n5 > 0.0;
}

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

void ValidateConfig(const CouplerPointConfig& config) {
    if (config.p <= 0) {
        throw std::invalid_argument("SolveCouplerPoint: p must be a positive integer.");
    }

    if (!IsFiniteNumber(config.a_over_A5) || config.a_over_A5 <= 0.0) {
        throw std::invalid_argument("SolveCouplerPoint: a_over_A5 must be a finite positive value.");
    }

    if (!IsFiniteNumber(config.c_over_a) || config.c_over_a < 0.0) {
        throw std::invalid_argument("SolveCouplerPoint: c_over_a must be a finite non-negative value.");
    }

    if (config.transverse_equation == CouplerTransverseEquation::kEq20) {
        if (!IsFiniteNumber(config.index_ratio_squared) ||
            !(config.index_ratio_squared > 0.0 && config.index_ratio_squared < 1.0)) {
            throw std::invalid_argument(
                "SolveCouplerPoint: Eq. (20) requires 0 < index_ratio_squared < 1."
            );
        }
    }

    const bool has_any_dimensional_input =
        (IsFiniteNumber(config.wavelength) && config.wavelength > 0.0) ||
        (IsFiniteNumber(config.n1) && config.n1 > 0.0) ||
        (IsFiniteNumber(config.n5) && config.n5 > 0.0);

    if (has_any_dimensional_input && !HasDimensionalInputs(config)) {
        throw std::invalid_argument(
            "SolveCouplerPoint: dimensional mode requires wavelength, n1 and n5 together."
        );
    }

    if (HasDimensionalInputs(config) && !(config.n1 > config.n5)) {
        throw std::invalid_argument(
            "SolveCouplerPoint: dimensional mode requires n1 > n5."
        );
    }

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

double SolveExactKxRatioEq6(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double p = static_cast<double>(config.p);

    const auto residual = [&](double u) {
        const double sqrt_term = std::sqrt(1.0 - Square(u));

        // Eq. (6), no limite simétrico n3 = n5, escrita em termos de
        // u = k_x A_5 / pi:
        //
        //   pi (a / A_5) u + 2 atan( u / sqrt(1-u^2) ) = p pi
        return kPi * s * u + 2.0 * std::atan(u / sqrt_term) - p * kPi;
    };

    return math::SolveRootByBisection(residual, kRootLowerBound, kRootUpperBound);
}

double SolveExactKxRatioEq20(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double r = config.index_ratio_squared;
    const double p = static_cast<double>(config.p);

    const auto residual = [&](double u) {
        const double sqrt_term = std::sqrt(1.0 - Square(u));

        // Eq. (20), no limite simétrico n3 = n5, escrita em termos de
        // u = k_x A_5 / pi:
        //
        //   pi (a / A_5) u + 2 atan( r u / sqrt(1-u^2) ) = p pi
        //   r = (n_5 / n_1)^2
        return kPi * s * u + 2.0 * std::atan(r * u / sqrt_term) - p * kPi;
    };

    return math::SolveRootByBisection(residual, kRootLowerBound, kRootUpperBound);
}

enum class RootSearchStatus {
    kFound,
    kBelowCutoff,
    kOutsideDomain
};

struct RootSearchResult {
    RootSearchStatus status = RootSearchStatus::kOutsideDomain;
    double root = NaN();
};

template <typename ResidualFunction>
RootSearchResult SolveRootWithStatus(const ResidualFunction& residual) {
    const double f_lower = residual(kRootLowerBound);
    const double f_upper = residual(kRootUpperBound);

    if (!IsFiniteNumber(f_lower) || !IsFiniteNumber(f_upper) || !(f_lower < 0.0)) {
        return {RootSearchStatus::kOutsideDomain, NaN()};
    }

    if (f_upper <= 0.0) {
        return {RootSearchStatus::kBelowCutoff, NaN()};
    }

    return {
        RootSearchStatus::kFound,
        math::SolveRootByBisection(residual, kRootLowerBound, kRootUpperBound)
    };
}

RootSearchResult SolveExactKxRatioWithStatus(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double p = static_cast<double>(config.p);

    if (config.transverse_equation == CouplerTransverseEquation::kEq6) {
        const auto residual = [&](double u) {
            const double sqrt_term = std::sqrt(1.0 - Square(u));
            return kPi * s * u + 2.0 * std::atan(u / sqrt_term) - p * kPi;
        };

        return SolveRootWithStatus(residual);
    }

    if (config.transverse_equation == CouplerTransverseEquation::kEq20) {
        const double r = config.index_ratio_squared;
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
        case CouplerTransverseEquation::kEq6:
            return SolveExactKxRatioEq6(config);
        case CouplerTransverseEquation::kEq20:
            return SolveExactKxRatioEq20(config);
    }

    throw std::invalid_argument("SolveCouplerPoint: unsupported transverse equation.");
}

void ResetDependentOutputs(CouplerPointResult& result) {
    result.transverse_root_found = false;
    result.kx_A5_over_pi = NaN();
    result.sqrt_one_minus_kx_A5_over_pi_squared = NaN();
    result.normalized_coupling = NaN();
    result.kx = NaN();
    result.kz = NaN();
    result.coupling_magnitude = NaN();
    result.full_transfer_length = NaN();
    result.dimensional_outputs_available = false;
}

void PopulateDimensionalOutputs(CouplerPointResult& result) {
    if (!HasDimensionalInputs(result.config)) {
        return;
    }

    result.k0 = 2.0 * kPi / result.config.wavelength;
    result.k1 = result.k0 * result.config.n1;
    result.k5 = result.k0 * result.config.n5;
    result.A5 = math::ComputeA(result.config.wavelength, result.config.n1, result.config.n5);
    result.a = result.config.a_over_A5 * result.A5;
    result.c = result.config.c_over_a * result.a;

    if (!result.transverse_root_found || !IsFiniteNumber(result.kx_A5_over_pi) ||
        !(result.A5 > 0.0)) {
        return;
    }

    result.kx = result.kx_A5_over_pi * kPi / result.A5;

    const double kz_squared = Square(result.k1) - Square(result.kx);
    if (!IsFiniteNumber(kz_squared) || kz_squared <= 0.0) {
        return;
    }

    result.kz = std::sqrt(kz_squared);

    if (!IsFiniteNumber(result.normalized_coupling) || !(result.a > 0.0)) {
        return;
    }

    const double refractive_factor =
        std::sqrt(1.0 - Square(result.config.n5 / result.config.n1));

    result.coupling_magnitude =
        result.normalized_coupling * refractive_factor * result.kz / result.a;

    if (IsFiniteNumber(result.coupling_magnitude) && result.coupling_magnitude > 0.0) {
        result.full_transfer_length = kPi / (2.0 * result.coupling_magnitude);
        result.dimensional_outputs_available = true;
        return;
    }

    if (IsFiniteNumber(result.coupling_magnitude) && result.coupling_magnitude == 0.0) {
        result.full_transfer_length = std::numeric_limits<double>::infinity();
        result.dimensional_outputs_available = true;
    }
}

double SolveClosedFormKxRatio(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;
    const double p = static_cast<double>(config.p);

    switch (config.transverse_equation) {
        case CouplerTransverseEquation::kEq6:
            // Eq. (12) no limite simétrico n3 = n5, em termos de u = k_x A_5 / pi.
            return p / (s + 2.0 / kPi);

        case CouplerTransverseEquation::kEq20: {
            const double r = config.index_ratio_squared;
            // Eq. (22) no limite simétrico n3 = n5, em termos de u = k_x A_5 / pi.
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

std::string ToString(CouplerTransverseEquation equation) {
    switch (equation) {
        case CouplerTransverseEquation::kEq6:
            return "eq6";
        case CouplerTransverseEquation::kEq20:
            return "eq20";
    }

    return "unknown";
}

CouplerTransverseEquation ParseCouplerTransverseEquation(const std::string& equation_text) {
    const std::string normalized = NormalizeEquationText(equation_text);

    if (normalized == "eq6" || normalized == "6") {
        return CouplerTransverseEquation::kEq6;
    }

    if (normalized == "eq20" || normalized == "20") {
        return CouplerTransverseEquation::kEq20;
    }

    throw std::invalid_argument(
        "ParseCouplerTransverseEquation: supported values are eq6 and eq20."
    );
}

CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config) {
    ValidateConfig(config);

    CouplerPointResult result;
    result.config = config;
    result.status = "ok";
    result.status_class = ClassifyStatus(result.status);
    result.equations_used = BuildEquationsUsed(config);
    result.a_over_A5 = config.a_over_A5;
    result.c_over_A5 = config.c_over_a * config.a_over_A5;
    ResetDependentOutputs(result);

    // O acoplador reutiliza a raiz transversal do guia único.
    // Aqui, "exact" significa resolver numericamente a equação transcendental
    // do modelo reduzido de Marcatili, não um problema vetorial completo de
    // supermodos em 2D.
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        const RootSearchResult root = SolveExactKxRatioWithStatus(config);
        if (root.status == RootSearchStatus::kOutsideDomain) {
            result.status = "outside_transverse_domain";
            result.status_class = ClassifyStatus(result.status);
            result.domain_valid = false;
            PopulateDimensionalOutputs(result);
            return result;
        }

        if (root.status == RootSearchStatus::kBelowCutoff) {
            result.status = "below_transverse_cutoff";
            result.status_class = ClassifyStatus(result.status);
            result.domain_valid = true;
            PopulateDimensionalOutputs(result);
            return result;
        }

        result.kx_A5_over_pi = root.root;
    } else {
        result.kx_A5_over_pi = SolveClosedFormKxRatio(config);
    }

    result.transverse_root_found = true;

    if (!(result.kx_A5_over_pi > 0.0 && result.kx_A5_over_pi < 1.0) ||
        !IsFiniteNumber(result.kx_A5_over_pi)) {
        result.status = "outside_transverse_domain";
        result.status_class = ClassifyStatus(result.status);
        result.domain_valid = false;
        ResetDependentOutputs(result);
        PopulateDimensionalOutputs(result);
        return result;
    }

    const double sqrt_argument = 1.0 - Square(result.kx_A5_over_pi);
    if (!(sqrt_argument > 0.0) || !IsFiniteNumber(sqrt_argument)) {
        result.status = "outside_transverse_domain";
        result.status_class = ClassifyStatus(result.status);
        result.domain_valid = false;
        ResetDependentOutputs(result);
        PopulateDimensionalOutputs(result);
        return result;
    }

    result.sqrt_one_minus_kx_A5_over_pi_squared = std::sqrt(sqrt_argument);

    // Implementação direta da forma normalizada da Eq. (34),
    // já escrita em função de:
    //
    //   u       = k_x A_5 / pi
    //   c / A_5 = (c / a) (a / A_5)
    //
    // Isso é conveniente para reproduzir diretamente as Figs. 10 e 11.
    result.normalized_coupling =
        2.0 * Square(result.kx_A5_over_pi) *
        result.sqrt_one_minus_kx_A5_over_pi_squared *
        std::exp(-kPi * result.c_over_A5 * result.sqrt_one_minus_kx_A5_over_pi_squared);

    // Acoplamento zero por underflow numérico em separações muito grandes
    // continua sendo um resultado fisicamente admissível.
    result.domain_valid =
        IsFiniteNumber(result.normalized_coupling) &&
        result.normalized_coupling >= 0.0;

    if (!result.domain_valid) {
        result.status = "outside_coupling_domain";
        result.status_class = ClassifyStatus(result.status);
        result.normalized_coupling = NaN();
        PopulateDimensionalOutputs(result);
        return result;
    }

    PopulateDimensionalOutputs(result);
    return result;
}

}  // namespace marcatili
