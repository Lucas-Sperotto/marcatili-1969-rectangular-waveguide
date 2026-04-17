#include "marcatili/physics/coupler.hpp"

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace marcatili {
namespace {

constexpr double kPi = 3.14159265358979323846;

double Square(double value) {
    return value * value;
}

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

double RootSolveByBisection(
    const std::function<double(double)>& function,
    double lower,
    double upper
) {
    double left = lower;
    double right = upper;
    double f_left = function(left);
    double f_right = function(right);

    if (f_left == 0.0) {
        return left;
    }

    if (f_right == 0.0) {
        return right;
    }

    if (f_left * f_right > 0.0) {
        throw std::runtime_error("Bisection interval does not bracket a root.");
    }

    for (int iteration = 0; iteration < 200; ++iteration) {
        const double midpoint = 0.5 * (left + right);
        const double f_mid = function(midpoint);

        if (std::abs(f_mid) < 1e-12 || std::abs(right - left) < 1e-12) {
            return midpoint;
        }

        if (f_left * f_mid <= 0.0) {
            right = midpoint;
            f_right = f_mid;
        } else {
            left = midpoint;
            f_left = f_mid;
        }
    }

    return 0.5 * (left + right);
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
    const double s = config.a_over_A5;

    if (config.transverse_equation == CouplerTransverseEquation::kEq6) {
        const auto equation_6 = [&](double u) {
            const double sqrt_term = std::sqrt(1.0 - Square(u));
            // Eq. (6) rewritten with n_3 = n_5 and u = k_x A_5 / pi:
            //   pi (a / A_5) u + 2 atan(u / sqrt(1-u^2)) = p pi.
            return kPi * s * u + 2.0 * std::atan(u / sqrt_term) -
                   static_cast<double>(config.p) * kPi;
        };

        return RootSolveByBisection(equation_6, 1e-12, 1.0 - 1e-12);
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

    return RootSolveByBisection(equation_20, 1e-12, 1.0 - 1e-12);
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

CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config) {
    ValidateConfig(config);

    CouplerPointResult result;
    result.config = config;
    result.status = "ok";
    result.a_over_A5 = config.a_over_A5;
    result.c_over_A5 = config.c_over_a * config.a_over_A5;

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

    // Eq. (34): normalized coupling written directly in terms of u = k_x A_5 / pi,
    // after substituting the penetration depth xi_5 from Eq. (8) or Eq. (18).
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
