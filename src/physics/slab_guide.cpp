#include "marcatili/physics/slab_guide.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

#include "marcatili/math/root_finding.hpp"
#include "marcatili/math/waveguide_math.hpp"

namespace marcatili {
namespace {

using math::ComputeA;
using math::kPi;
using math::PenetrationDepth;
using math::SafeUpperBound;
using math::Square;

constexpr double kRootLowerBound = 1e-12;

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

void ResetDependentOutputs(SingleGuideResult& result) {
    result.ky = NaN();
    result.kz = NaN();
    result.eta2 = NaN();
    result.eta4 = NaN();
    result.kz_normalized_against_n4 = NaN();
    result.domain_valid = false;
    result.guided = false;
}

void InvalidateResult(SingleGuideResult& result, const char* status) {
    result.status = status;
    ResetDependentOutputs(result);
}

void ValidateConfig(const SingleGuideConfig& config) {
    if (!IsFiniteNumber(config.wavelength) || config.wavelength <= 0.0) {
        throw std::invalid_argument(
            "SolveSlabGuide: wavelength must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.b) || config.b <= 0.0) {
        throw std::invalid_argument(
            "SolveSlabGuide: b must be a finite positive value for the slab model."
        );
    }

    // Mantemos p e q positivos por uniformidade com a interface do guia único,
    // embora, no limite de lâmina, apenas q participe da quantização transversal.
    if (config.p <= 0 || config.q <= 0) {
        throw std::invalid_argument(
            "SolveSlabGuide: mode indices p and q must be positive integers."
        );
    }

    if (!IsFiniteNumber(config.n1) || config.n1 <= 0.0 ||
        !IsFiniteNumber(config.n2) || config.n2 <= 0.0 ||
        !IsFiniteNumber(config.n4) || config.n4 <= 0.0) {
        throw std::invalid_argument(
            "SolveSlabGuide: n1, n2 and n4 must be finite positive values."
        );
    }

    if (!(config.n1 > std::max(config.n2, config.n4))) {
        throw std::invalid_argument(
            "SolveSlabGuide: requires n1 > n2 and n1 > n4."
        );
    }
}

SingleGuideResult BuildBaseResult(const SingleGuideConfig& config) {
    SingleGuideResult result;
    result.config = config;
    result.status = "ok";

    result.k0 = 2.0 * kPi / config.wavelength;
    result.k1 = result.k0 * config.n1;
    result.k2 = result.k0 * config.n2;
    result.k3 = NaN();
    result.k4 = result.k0 * config.n4;
    result.k5 = NaN();

    result.A2 = ComputeA(config.wavelength, config.n1, config.n2);
    result.A3 = NaN();
    result.A4 = ComputeA(config.wavelength, config.n1, config.n4);
    result.A5 = NaN();

    // No limite de lâmina não há quantização independente em x.
    result.kx = 0.0;
    result.xi3 = NaN();
    result.xi5 = NaN();

    result.approximation_checks.kx_a3_over_pi_squared = 0.0;
    result.approximation_checks.kx_a5_over_pi_squared = 0.0;

    result.b_over_A4 = config.b / result.A4;

    return result;
}

void FinalizeResult(SingleGuideResult& result) {
    const double denominator = Square(result.k1) - Square(result.k4);

    result.kz_normalized_against_n4 =
        (denominator > 0.0 && IsFiniteNumber(result.kz))
            ? (Square(result.kz) - Square(result.k4)) / denominator
            : NaN();

    result.guided =
        IsFiniteNumber(result.kz) &&
        Square(result.kz) > Square(result.k4) &&
        Square(result.kz) <= Square(result.k1) &&
        IsFiniteNumber(result.kz_normalized_against_n4) &&
        result.kz_normalized_against_n4 >= 0.0 &&
        result.kz_normalized_against_n4 <= 1.0;

    result.domain_valid = true;

    if (!result.guided && result.status == "ok") {
        result.status = "below_cutoff";
    }
}

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

}  // namespace

SingleGuideResult SolveSlabGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    if (config.family == SingleGuideFamily::kEy) {
        // Limite 1D da família E_y: mantém-se apenas a quantização em y
        // com a correção de fase correspondente.
        const double y_denominator =
            1.0 +
            (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
                (kPi * Square(config.n1) * config.b);

        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;
        result.equations_used = "slab limit of (13), (14), (16)";
    } else {
        // Limite 1D da família E_x.
        const double y_denominator =
            1.0 + (result.A2 + result.A4) / (kPi * config.b);

        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;
        result.equations_used = "slab limit of (23), (24), (26)";
    }

    result.approximation_checks.ky_a2_over_pi_squared =
        Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);

    const double kz_squared =
        Square(result.k1) - Square(result.ky);

    const double eta2_argument =
        1.0 - result.approximation_checks.ky_a2_over_pi_squared;
    const double eta4_argument =
        1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    const bool domain_valid =
        IsFiniteNumber(kz_squared) && kz_squared >= 0.0 &&
        IsFiniteNumber(eta2_argument) && eta2_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!domain_valid) {
        InvalidateResult(result, "outside_closed_form_domain");
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.eta2 = (result.A2 / kPi) / std::sqrt(eta2_argument);
    result.eta4 = (result.A4 / kPi) / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

SingleGuideResult SolveSlabGuideExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    const double lower_bound = kRootLowerBound;
    const double upper_bound = SafeUpperBound(kPi / result.A2, kPi / result.A4);

    std::function<double(double)> characteristic_function;

    if (config.family == SingleGuideFamily::kEy) {
        result.equations_used = "slab limit of (7), (9), (10)";

        characteristic_function = [&](double ky_value) {
            const double eta2 = PenetrationDepth(result.A2, ky_value);
            const double eta4 = PenetrationDepth(result.A4, ky_value);

            return ky_value * config.b +
                   std::atan((Square(config.n2) / Square(config.n1)) * ky_value * eta2) +
                   std::atan((Square(config.n4) / Square(config.n1)) * ky_value * eta4) -
                   static_cast<double>(config.q) * kPi;
        };
    } else {
        result.equations_used = "slab limit of (21), (19), (10)";

        characteristic_function = [&](double ky_value) {
            const double eta2 = PenetrationDepth(result.A2, ky_value);
            const double eta4 = PenetrationDepth(result.A4, ky_value);

            return ky_value * config.b +
                   std::atan(ky_value * eta2) +
                   std::atan(ky_value * eta4) -
                   static_cast<double>(config.q) * kPi;
        };
    }

    const double f_lower = characteristic_function(lower_bound);
    const double f_upper = characteristic_function(upper_bound);

    if (!IsFiniteNumber(f_lower) || !IsFiniteNumber(f_upper) || !(f_lower < 0.0)) {
        InvalidateResult(result, "outside_exact_domain");
        return result;
    }

    // Quando a função continua não-positiva até o limite superior permitido,
    // o modo permanece abaixo do cutoff dentro do intervalo físico admissível.
    if (f_upper <= 0.0) {
        InvalidateResult(result, "below_cutoff");
        return result;
    }

    if (!BracketsRoot(characteristic_function, lower_bound, upper_bound)) {
        InvalidateResult(result, "outside_exact_domain");
        return result;
    }

    result.ky = math::SolveRootByBisection(
        characteristic_function,
        lower_bound,
        upper_bound
    );

    result.approximation_checks.ky_a2_over_pi_squared =
        Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);

    const double kz_squared =
        Square(result.k1) - Square(result.ky);

    const double eta2_argument =
        Square(kPi / result.A2) - Square(result.ky);
    const double eta4_argument =
        Square(kPi / result.A4) - Square(result.ky);

    const bool domain_valid =
        IsFiniteNumber(kz_squared) && kz_squared >= 0.0 &&
        IsFiniteNumber(eta2_argument) && eta2_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!domain_valid) {
        InvalidateResult(result, "outside_exact_domain");
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.eta2 = 1.0 / std::sqrt(eta2_argument);
    result.eta4 = 1.0 / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

SingleGuideResult SolveSlabGuide(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveSlabGuideExact(config);
    }

    return SolveSlabGuideClosedForm(config);
}

}  // namespace marcatili
