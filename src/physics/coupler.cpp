#include "marcatili/physics/coupler.hpp"

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

#include "marcatili/math/root_finding.hpp"
#include "marcatili/math/waveguide_math.hpp"

namespace marcatili {
namespace {

using math::kPi;
using math::Square;

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

void ValidateConfig(const CouplerPointConfig& config) {
    if (config.p <= 0) {
        throw std::runtime_error("p must be a positive integer.");
    }

    if (config.a_over_A5 <= 0.0) {
        throw std::runtime_error("a_over_A5 must be positive.");
    }

    if (config.c_over_a < 0.0) {
        throw std::runtime_error("c_over_a must be non-negative.");
    }

    if (config.transverse_equation == CouplerTransverseEquation::kEq20 &&
        !(config.index_ratio_squared > 0.0 && config.index_ratio_squared < 1.0)) {
        throw std::runtime_error(
            "Eq. (20) requires a valid index_ratio_squared = (n5/n1)^2 between 0 and 1."
        );
    }
}

double SolveExactKxRatio(const CouplerPointConfig& config) {
    // We solve directly for u = k_x A_5 / pi on the unit interval used by Eq. (34).
    // This keeps the coupler code in the same normalized variables as the paper,
    // which is convenient for plotting Fig. 10 and Fig. 11 directly.
    const double s = config.a_over_A5;

    if (config.transverse_equation == CouplerTransverseEquation::kEq6) {
        const auto equation_6 = [&](double u) {
            const double sqrt_term = std::sqrt(1.0 - Square(u));
            // Eq. (6) rewritten with n_3 = n_5 and u = k_x A_5 / pi:
            //   pi (a / A_5) u + 2 atan(u / sqrt(1-u^2)) = p pi.
            return kPi * s * u + 2.0 * std::atan(u / sqrt_term) -
                   static_cast<double>(config.p) * kPi;
        };

        return math::SolveRootByBisection(equation_6, 1e-12, 1.0 - 1e-12);
    }

    const double r = config.index_ratio_squared;
    const auto equation_20 = [&](double u) {
        const double sqrt_term = std::sqrt(1.0 - Square(u));
        // Eq. (20) rewritten with n_3 = n_5 and u = k_x A_5 / pi:
        //   pi (a / A_5) u + 2 atan(r u / sqrt(1-u^2)) = p pi,
        // where r = (n_5 / n_1)^2.
        return kPi * s * u + 2.0 * std::atan(r * u / sqrt_term) -
               static_cast<double>(config.p) * kPi;
    };

    return math::SolveRootByBisection(equation_20, 1e-12, 1.0 - 1e-12);
}

double SolveClosedFormKxRatio(const CouplerPointConfig& config) {
    const double s = config.a_over_A5;

    if (config.transverse_equation == CouplerTransverseEquation::kEq6) {
        // Eq. (12) in the symmetric n_3 = n_5 limit, expressed as u = k_x A_5 / pi.
        return static_cast<double>(config.p) / (s + 2.0 / kPi);
    }

    const double r = config.index_ratio_squared;
    // Eq. (22) in the symmetric n_3 = n_5 limit, expressed as u = k_x A_5 / pi.
    return static_cast<double>(config.p) / (s + 2.0 * r / kPi);
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
    if (equation_text == "eq6" || equation_text == "Eq6" ||
        equation_text == "6" || equation_text == "Eq. (6)") {
        return CouplerTransverseEquation::kEq6;
    }

    if (equation_text == "eq20" || equation_text == "Eq20" ||
        equation_text == "20" || equation_text == "Eq. (20)") {
        return CouplerTransverseEquation::kEq20;
    }

    throw std::runtime_error(
        "Unsupported transverse_equation. Use one of: eq6, eq20."
    );
}

/**
 * @brief Solves a single point for the directional coupler problem.
 * @details This function implements the normalized coupling model from Eq. (34)
 * of Marcatili's paper. It first calculates the normalized transverse wave
 * number `u = kx * A5 / pi` for an isolated guide, using either the `exact`
 * (numerical root of Eq. 6 or 20) or `closed_form` (algebraic approximation
 * from Eq. 12 or 22) solver. It then uses this `u` to compute the normalized
 * coupling coefficient.
 * @param config The configuration for the coupler point calculation.
 * @return A struct containing the calculated results.
 */
CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config) {
    ValidateConfig(config);

    CouplerPointResult result;
    result.config = config;
    result.status = "ok";
    result.a_over_A5 = config.a_over_A5;
    result.c_over_A5 = config.c_over_a * config.a_over_A5;

    // The coupler reuses the single-guide transverse root. In other words,
    // the coupling calculation is layered on top of the same modal model,
    // either with a closed-form transverse estimate or with a numerical root.
    result.kx_A5_over_pi =
        config.solver_model == SingleGuideSolverModel::kExact
            ? SolveExactKxRatio(config)
            : SolveClosedFormKxRatio(config);

    if (!(result.kx_A5_over_pi > 0.0 && result.kx_A5_over_pi < 1.0)) {
        result.status = "outside_transverse_domain";
        result.domain_valid = false;
        result.kx_A5_over_pi = NaN();
        result.sqrt_one_minus_kx_A5_over_pi_squared = NaN();
        result.normalized_coupling = NaN();
        return result;
    }

    result.sqrt_one_minus_kx_A5_over_pi_squared =
        std::sqrt(1.0 - Square(result.kx_A5_over_pi));

    // Direct implementation of the normalized coupling coefficient from Eq. (34).
    // The equation is expressed in terms of the normalized transverse wave
    // number u = k_x * A_5 / pi and the normalized separation c / A_5. This
    // form is convenient for reproducing Fig. 10 and Fig. 11.
    //
    // after substituting the penetration depth xi_5 from Eq. (8) or Eq. (18).
    // At this stage the repository stays in the normalized coupling domain;
    // dimensional K and full coupling length L are documented but not yet the
    // main API returned by this executable.
    result.normalized_coupling =
        2.0 * Square(result.kx_A5_over_pi) *
        result.sqrt_one_minus_kx_A5_over_pi_squared *
        std::exp(-kPi * result.c_over_A5 * result.sqrt_one_minus_kx_A5_over_pi_squared);

    result.domain_valid = std::isfinite(result.normalized_coupling) &&
                          result.normalized_coupling > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_coupling_domain";
        result.normalized_coupling = NaN();
        return result;
    }

    if (config.solver_model == SingleGuideSolverModel::kExact) {
        // "Exact" again means: exact root of the transcendental equation chosen
        // for the reduced coupler model, not a full supermode solve of two guides.
        result.equations_used =
            config.transverse_equation == CouplerTransverseEquation::kEq6
                ? "(6), (8), (33), (34)"
                : "(20), (18), (33), (34)";
    } else {
        result.equations_used =
            config.transverse_equation == CouplerTransverseEquation::kEq6
                ? "(12), (34)"
                : "(22), (34)";
    }

    return result;
}

}  // namespace marcatili
