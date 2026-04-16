#include "marcatili/physics/slab_guide.hpp"

#include <algorithm>
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

double ComputeA(double wavelength, double n1, double nv) {
    return wavelength / (2.0 * std::sqrt(Square(n1) - Square(nv)));
}

double RootSolveByBisection(
    const std::function<double(double)>& function,
    double lower,
    double upper
) {
    double left = lower;
    double right = upper;
    double f_left = function(left);

    for (int iteration = 0; iteration < 200; ++iteration) {
        const double midpoint = 0.5 * (left + right);
        const double f_mid = function(midpoint);

        if (std::abs(f_mid) < 1e-12 || std::abs(right - left) < 1e-12) {
            return midpoint;
        }

        if (f_left * f_mid <= 0.0) {
            right = midpoint;
        } else {
            left = midpoint;
            f_left = f_mid;
        }
    }

    return 0.5 * (left + right);
}

double SafeUpperBound(double a_value, double b_value) {
    return std::min(a_value, b_value) * (1.0 - 1e-10);
}

double PenetrationDepth(double A_value, double transverse_wave_number) {
    return 1.0 / std::sqrt(Square(kPi / A_value) - Square(transverse_wave_number));
}

void ValidateConfig(const SingleGuideConfig& config) {
    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.b <= 0.0) {
        throw std::runtime_error("b must be positive for the slab model.");
    }

    if (config.p <= 0 || config.q <= 0) {
        throw std::runtime_error("mode indices p and q must be positive integers.");
    }

    if (config.n1 <= std::max(config.n2, config.n4)) {
        throw std::runtime_error("The slab model requires n1 > n2 and n1 > n4.");
    }
}

SingleGuideResult BuildBaseResult(const SingleGuideConfig& config) {
    SingleGuideResult result;
    result.config = config;
    result.status = "ok";
    result.k0 = 2.0 * kPi / config.wavelength;
    result.k1 = result.k0 * config.n1;
    result.k2 = result.k0 * config.n2;
    result.k3 = result.k0 * config.n3;
    result.k4 = result.k0 * config.n4;
    result.k5 = result.k0 * config.n5;
    result.A2 = ComputeA(config.wavelength, config.n1, config.n2);
    result.A3 = ComputeA(config.wavelength, config.n1, config.n3);
    result.A4 = ComputeA(config.wavelength, config.n1, config.n4);
    result.A5 = ComputeA(config.wavelength, config.n1, config.n5);
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
        denominator > 0.0 ? (Square(result.kz) - Square(result.k4)) / denominator : NaN();

    result.guided =
        Square(result.kz) > Square(result.k4) &&
        Square(result.kz) <= Square(result.k1) &&
        result.kz_normalized_against_n4 >= 0.0 &&
        result.kz_normalized_against_n4 <= 1.0;

    if (!result.guided && result.status == "ok") {
        result.status = "below_cutoff";
    }
}

}  // namespace

SingleGuideResult SolveSlabGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    if (config.family == SingleGuideFamily::kEy) {
        const double y_denominator =
            1.0 +
            (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
                (kPi * Square(config.n1) * config.b);
        result.ky = (config.q * kPi / config.b) / y_denominator;
        result.equations_used = "slab limit of (13), (14), (16)";
    } else {
        const double y_denominator = 1.0 + (result.A2 + result.A4) / (kPi * config.b);
        result.ky = (config.q * kPi / config.b) / y_denominator;
        result.equations_used = "slab limit of (23), (24), (26)";
    }

    result.approximation_checks.ky_a2_over_pi_squared = Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared = Square(result.ky * result.A4 / kPi);

    const double kz_squared = Square(result.k1) - Square(result.ky);
    const double eta2_argument = 1.0 - result.approximation_checks.ky_a2_over_pi_squared;
    const double eta4_argument = 1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    result.domain_valid = kz_squared >= 0.0 && eta2_argument > 0.0 && eta4_argument > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_closed_form_domain";
        result.kz = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.kz_normalized_against_n4 = NaN();
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

    const auto upper_bound = SafeUpperBound(kPi / result.A2, kPi / result.A4);
    const auto lower_bound = 1e-12;

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

    if (!(f_lower < 0.0)) {
        result.status = "outside_exact_domain";
        result.domain_valid = false;
        result.ky = NaN();
        result.kz = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.kz_normalized_against_n4 = NaN();
        return result;
    }

    if (f_upper <= 0.0) {
        result.status = "below_cutoff";
        result.domain_valid = false;
        result.ky = NaN();
        result.kz = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.kz_normalized_against_n4 = NaN();
        return result;
    }

    result.ky = RootSolveByBisection(characteristic_function, lower_bound, upper_bound);

    result.approximation_checks.ky_a2_over_pi_squared = Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared = Square(result.ky * result.A4 / kPi);

    const double kz_squared = Square(result.k1) - Square(result.ky);
    const double eta2_argument = Square(kPi / result.A2) - Square(result.ky);
    const double eta4_argument = Square(kPi / result.A4) - Square(result.ky);

    result.domain_valid = kz_squared >= 0.0 && eta2_argument > 0.0 && eta4_argument > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_exact_domain";
        result.kz = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.kz_normalized_against_n4 = NaN();
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
