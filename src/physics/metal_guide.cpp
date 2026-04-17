#include "marcatili/physics/metal_guide.hpp"

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

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

void ResetDependentOutputs(SingleGuideResult& result) {
    result.kz = NaN();
    result.xi3 = NaN();
    result.xi5 = NaN();
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
            "SolveMetalGuide: wavelength must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.a) || !IsFiniteNumber(config.b) ||
        config.a <= 0.0 || config.b <= 0.0) {
        throw std::invalid_argument(
            "SolveMetalGuide: a and b must be finite positive values."
        );
    }

    if (config.p <= 0 || config.q <= 0) {
        throw std::invalid_argument(
            "SolveMetalGuide: mode indices p and q must be positive integers."
        );
    }

    if (!IsFiniteNumber(config.n1) || !IsFiniteNumber(config.n4) ||
        config.n1 <= 0.0 || config.n4 <= 0.0) {
        throw std::invalid_argument(
            "SolveMetalGuide: n1 and n4 must be finite positive values."
        );
    }

    if (!(config.n1 > config.n4)) {
        throw std::invalid_argument(
            "SolveMetalGuide: the current metal-guide model requires n1 > n4."
        );
    }
}

SingleGuideResult BuildBaseResult(const SingleGuideConfig& config) {
    SingleGuideResult result;
    result.config = config;
    result.status = "ok";

    result.k0 = 2.0 * kPi / config.wavelength;
    result.k1 = result.k0 * config.n1;

    // No modelo operacional atual do guia metalizado, as regiões externas
    // são colapsadas para o mesmo índice n4.
    result.k2 = NaN();
    result.k3 = result.k0 * config.n4;
    result.k4 = result.k0 * config.n4;
    result.k5 = result.k0 * config.n4;

    result.A2 = NaN();
    result.A3 = ComputeA(config.wavelength, config.n1, config.n4);
    result.A4 = ComputeA(config.wavelength, config.n1, config.n4);
    result.A5 = ComputeA(config.wavelength, config.n1, config.n4);

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

SingleGuideResult SolveMetalGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    if (config.family == SingleGuideFamily::kEy) {
        // Variante aproximada para E_y no caso metalizado:
        // - n3 = n5 = n4;
        // - a fronteira superior é substituída por uma condição tipo PEC,
        //   modelada como termo de fase pi/2 na direção y.
        const double x_denominator =
            1.0 + (result.A3 + result.A5) / (kPi * config.a);

        const double y_denominator =
            1.0 +
            (Square(config.n4) * result.A4) /
                (kPi * Square(config.n1) * config.b);

        result.kx = (static_cast<double>(config.p) * kPi / config.a) / x_denominator;
        result.ky =
            ((static_cast<double>(config.q) - 0.5) * kPi / config.b) / y_denominator;

        result.equations_used =
            "Fig. 8 metal-boundary closed-form model for E_y, adapted from (6), (7), (12), (13) "
            "with n3=n5=n4 and a PEC-like phase term pi/2 at the top boundary";
    } else {
        // Variante aproximada para E_x no caso metalizado:
        // - a condição PEC anula o campo tangencial no topo;
        // - em y, mantém-se apenas o termo dielétrico inferior.
        const double x_denominator =
            1.0 +
            (Square(config.n4) * (result.A3 + result.A5)) /
                (kPi * Square(config.n1) * config.a);

        const double y_denominator =
            1.0 + result.A4 / (kPi * config.b);

        result.kx = (static_cast<double>(config.p) * kPi / config.a) / x_denominator;
        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;

        result.equations_used =
            "Fig. 8 metal-boundary closed-form model for E_x, adapted from (20), (21), (22), (23) "
            "with n3=n5=n4, a PEC top boundary and only the lower dielectric term kept in y";
    }

    result.approximation_checks.kx_a3_over_pi_squared =
        Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared =
        Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared = NaN();
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);

    const double kz_squared =
        Square(result.k1) - Square(result.kx) - Square(result.ky);

    const double xi_argument =
        1.0 - result.approximation_checks.kx_a3_over_pi_squared;

    const double eta4_argument =
        1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    const bool domain_valid =
        IsFiniteNumber(kz_squared) && kz_squared >= 0.0 &&
        IsFiniteNumber(xi_argument) && xi_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!domain_valid) {
        InvalidateResult(result, "outside_closed_form_domain");
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.xi3 = (result.A3 / kPi) / std::sqrt(xi_argument);
    result.xi5 = (result.A5 / kPi) / std::sqrt(xi_argument);
    result.eta2 = NaN();
    result.eta4 = (result.A4 / kPi) / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

SingleGuideResult SolveMetalGuideExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    std::function<double(double)> fx;
    std::function<double(double)> fy;

    if (config.family == SingleGuideFamily::kEy) {
        fx = [&](double kx_value) {
            const double xi = PenetrationDepth(result.A4, kx_value);
            return kx_value * config.a +
                   2.0 * std::atan(kx_value * xi) -
                   static_cast<double>(config.p) * kPi;
        };

        fy = [&](double ky_value) {
            const double eta4 = PenetrationDepth(result.A4, ky_value);
            return ky_value * config.b +
                   std::atan((Square(config.n4) / Square(config.n1)) * ky_value * eta4) -
                   (static_cast<double>(config.q) - 0.5) * kPi;
        };
    } else {
        fx = [&](double kx_value) {
            const double xi = PenetrationDepth(result.A4, kx_value);
            return kx_value * config.a +
                   2.0 * std::atan((Square(config.n4) / Square(config.n1)) * kx_value * xi) -
                   static_cast<double>(config.p) * kPi;
        };

        fy = [&](double ky_value) {
            const double eta4 = PenetrationDepth(result.A4, ky_value);
            return ky_value * config.b +
                   std::atan(ky_value * eta4) -
                   static_cast<double>(config.q) * kPi;
        };
    }

    const double lower = 1e-12;
    const double x_upper = SafeUpperBound(kPi / result.A3, kPi / result.A5);
    const double y_upper = SafeUpperBound(kPi / result.A4, kPi / result.A4);

    // Aqui "exact" significa resolver numericamente o sistema transcendental
    // do modelo reduzido metalizado, não o problema vetorial completo.
    if (!BracketsRoot(fx, lower, x_upper) || !BracketsRoot(fy, lower, y_upper)) {
        result.kx = NaN();
        result.ky = NaN();
        InvalidateResult(result, "outside_exact_domain");
        return result;
    }

    result.kx = math::SolveRootByBisection(fx, lower, x_upper);
    result.ky = math::SolveRootByBisection(fy, lower, y_upper);

    if (config.family == SingleGuideFamily::kEy) {
        result.equations_used =
            "Fig. 8 metal-boundary exact model for E_y, based on the reduced transcendental system "
            "adapted from (6), (7) and Appendix A.1 with n3=n5=n4 and a PEC top boundary";
    } else {
        result.equations_used =
            "Fig. 8 metal-boundary exact model for E_x, based on the reduced transcendental system "
            "adapted from (20), (21), (18), (19) with n3=n5=n4, a PEC top boundary and a single lower dielectric term in y";
    }

    result.approximation_checks.kx_a3_over_pi_squared =
        Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared =
        Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared = NaN();
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);

    const double kz_squared =
        Square(result.k1) - Square(result.kx) - Square(result.ky);

    const double xi_argument =
        Square(kPi / result.A4) - Square(result.kx);

    const double eta4_argument =
        Square(kPi / result.A4) - Square(result.ky);

    const bool domain_valid =
        IsFiniteNumber(kz_squared) && kz_squared >= 0.0 &&
        IsFiniteNumber(xi_argument) && xi_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!domain_valid) {
        InvalidateResult(result, "outside_exact_domain");
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.xi3 = 1.0 / std::sqrt(xi_argument);
    result.xi5 = 1.0 / std::sqrt(xi_argument);
    result.eta2 = NaN();
    result.eta4 = 1.0 / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

SingleGuideResult SolveMetalGuide(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveMetalGuideExact(config);
    }

    return SolveMetalGuideClosedForm(config);
}

}  // namespace marcatili
