#include "marcatili/physics/single_guide.hpp"

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

double SafeUpperBound(double a_value, double b_value) {
    return std::min(a_value, b_value) * (1.0 - 1e-10);
}

double PenetrationDepth(double A_value, double transverse_wave_number) {
    return 1.0 / std::sqrt(Square(kPi / A_value) - Square(transverse_wave_number));
}

double SolveExactRoot(
    const std::function<double(double)>& function,
    double upper_bound
) {
    const double lower_bound = 1e-12;
    const double f_lower = function(lower_bound);
    const double f_upper = function(upper_bound);

    if (!(f_lower < 0.0 && f_upper > 0.0)) {
        throw std::runtime_error("No guided root found in the allowed transverse-wave-number interval.");
    }

    return RootSolveByBisection(function, lower_bound, upper_bound);
}

void ValidateConfig(const SingleGuideConfig& config) {
    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.a <= 0.0 || config.b <= 0.0) {
        throw std::runtime_error("a and b must be positive.");
    }

    if (config.p <= 0 || config.q <= 0) {
        throw std::runtime_error("mode indices p and q must be positive integers.");
    }

    const double external_max =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));

    if (config.n1 <= external_max) {
        throw std::runtime_error("The current closed-form model requires n1 > n2, n3, n4, n5.");
    }
}

}  // namespace

std::string ToString(SingleGuideFamily family) {
    switch (family) {
        case SingleGuideFamily::kEy:
            return "E_y";
        case SingleGuideFamily::kEx:
            return "E_x";
    }

    return "unknown";
}

std::string ToString(SingleGuideSolverModel solver_model) {
    switch (solver_model) {
        case SingleGuideSolverModel::kClosedForm:
            return "closed_form";
        case SingleGuideSolverModel::kExact:
            return "exact";
    }

    return "unknown";
}

SingleGuideFamily ParseSingleGuideFamily(const std::string& family_text) {
    if (family_text == "E_y" || family_text == "Ey" || family_text == "E^y") {
        return SingleGuideFamily::kEy;
    }

    if (family_text == "E_x" || family_text == "Ex" || family_text == "E^x") {
        return SingleGuideFamily::kEx;
    }

    throw std::runtime_error(
        "Unsupported mode_family. Use one of: E_y, Ey, E^y, E_x, Ex, E^x."
    );
}

SingleGuideSolverModel ParseSingleGuideSolverModel(const std::string& solver_model_text) {
    if (solver_model_text == "closed_form" || solver_model_text == "approx" ||
        solver_model_text == "approximate") {
        return SingleGuideSolverModel::kClosedForm;
    }

    if (solver_model_text == "exact" || solver_model_text == "transcendental") {
        return SingleGuideSolverModel::kExact;
    }

    throw std::runtime_error(
        "Unsupported solver_model. Use one of: closed_form, exact."
    );
}

SingleGuideResult SolveSingleGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

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

    if (config.family == SingleGuideFamily::kEy) {
        const double x_denominator = 1.0 + (result.A3 + result.A5) / (kPi * config.a);
        const double y_denominator =
            1.0 +
            (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
                (kPi * Square(config.n1) * config.b);

        result.kx = (config.p * kPi / config.a) / x_denominator;
        result.ky = (config.q * kPi / config.b) / y_denominator;
        result.equations_used = "(10), (12), (13), (14), (15), (16)";
    } else {
        const double x_denominator =
            1.0 +
            (Square(config.n3) * result.A3 + Square(config.n5) * result.A5) /
                (kPi * Square(config.n1) * config.a);
        const double y_denominator = 1.0 + (result.A2 + result.A4) / (kPi * config.b);

        result.kx = (config.p * kPi / config.a) / x_denominator;
        result.ky = (config.q * kPi / config.b) / y_denominator;
        result.equations_used = "(10), (22), (23), (24), (25), (26)";
    }

    result.approximation_checks.kx_a3_over_pi_squared = Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared = Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared = Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared = Square(result.ky * result.A4 / kPi);

    const double kz_squared = Square(result.k1) - Square(result.kx) - Square(result.ky);
    const double xi3_argument = 1.0 - result.approximation_checks.kx_a3_over_pi_squared;
    const double xi5_argument = 1.0 - result.approximation_checks.kx_a5_over_pi_squared;
    const double eta2_argument = 1.0 - result.approximation_checks.ky_a2_over_pi_squared;
    const double eta4_argument = 1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    result.domain_valid =
        kz_squared >= 0.0 &&
        xi3_argument > 0.0 &&
        xi5_argument > 0.0 &&
        eta2_argument > 0.0 &&
        eta4_argument > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_closed_form_domain";
        result.kz = NaN();
        result.xi3 = NaN();
        result.xi5 = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.b_over_A4 = config.b / result.A4;
        result.kz_normalized_against_n4 = NaN();
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.xi3 = (result.A3 / kPi) / std::sqrt(xi3_argument);
    result.xi5 = (result.A5 / kPi) / std::sqrt(xi5_argument);
    result.eta2 = (result.A2 / kPi) / std::sqrt(eta2_argument);
    result.eta4 = (result.A4 / kPi) / std::sqrt(eta4_argument);
    result.b_over_A4 = config.b / result.A4;

    const double denominator = Square(result.k1) - Square(result.k4);
    result.kz_normalized_against_n4 =
        denominator > 0.0 ? (Square(result.kz) - Square(result.k4)) / denominator : NaN();

    result.guided =
        Square(result.kz) > Square(result.k4) &&
        Square(result.kz) <= Square(result.k1) &&
        result.kz_normalized_against_n4 >= 0.0 &&
        result.kz_normalized_against_n4 <= 1.0;

    if (!result.guided) {
        result.status = "below_cutoff";
    }

    return result;
}

SingleGuideResult SolveSingleGuideExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

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

    try {
        if (config.family == SingleGuideFamily::kEy) {
            const auto fx = [&](double kx_value) {
                const double xi3 = PenetrationDepth(result.A3, kx_value);
                const double xi5 = PenetrationDepth(result.A5, kx_value);
                return kx_value * config.a +
                       std::atan(kx_value * xi3) +
                       std::atan(kx_value * xi5) -
                       static_cast<double>(config.p) * kPi;
            };

            const auto fy = [&](double ky_value) {
                const double eta2 = PenetrationDepth(result.A2, ky_value);
                const double eta4 = PenetrationDepth(result.A4, ky_value);
                return ky_value * config.b +
                       std::atan((Square(config.n2) / Square(config.n1)) * ky_value * eta2) +
                       std::atan((Square(config.n4) / Square(config.n1)) * ky_value * eta4) -
                       static_cast<double>(config.q) * kPi;
            };

            result.kx = SolveExactRoot(
                fx,
                SafeUpperBound(kPi / result.A3, kPi / result.A5)
            );
            result.ky = SolveExactRoot(
                fy,
                SafeUpperBound(kPi / result.A2, kPi / result.A4)
            );
            result.equations_used = "(6), (7), (8), (9), (10)";
        } else {
            const auto fx = [&](double kx_value) {
                const double xi3 = PenetrationDepth(result.A3, kx_value);
                const double xi5 = PenetrationDepth(result.A5, kx_value);
                return kx_value * config.a +
                       std::atan((Square(config.n3) / Square(config.n1)) * kx_value * xi3) +
                       std::atan((Square(config.n5) / Square(config.n1)) * kx_value * xi5) -
                       static_cast<double>(config.p) * kPi;
            };

            const auto fy = [&](double ky_value) {
                const double eta2 = PenetrationDepth(result.A2, ky_value);
                const double eta4 = PenetrationDepth(result.A4, ky_value);
                return ky_value * config.b +
                       std::atan(ky_value * eta2) +
                       std::atan(ky_value * eta4) -
                       static_cast<double>(config.q) * kPi;
            };

            result.kx = SolveExactRoot(
                fx,
                SafeUpperBound(kPi / result.A3, kPi / result.A5)
            );
            result.ky = SolveExactRoot(
                fy,
                SafeUpperBound(kPi / result.A2, kPi / result.A4)
            );
            result.equations_used = "(20), (21), (18), (19), (10)";
        }
    } catch (const std::runtime_error&) {
        result.status = "outside_exact_domain";
        result.domain_valid = false;
        result.guided = false;
        result.kx = NaN();
        result.ky = NaN();
        result.kz = NaN();
        result.xi3 = NaN();
        result.xi5 = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.b_over_A4 = config.b / result.A4;
        result.kz_normalized_against_n4 = NaN();
        return result;
    }

    result.approximation_checks.kx_a3_over_pi_squared = Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared = Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared = Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared = Square(result.ky * result.A4 / kPi);

    const double kz_squared = Square(result.k1) - Square(result.kx) - Square(result.ky);
    const double xi3_argument = Square(kPi / result.A3) - Square(result.kx);
    const double xi5_argument = Square(kPi / result.A5) - Square(result.kx);
    const double eta2_argument = Square(kPi / result.A2) - Square(result.ky);
    const double eta4_argument = Square(kPi / result.A4) - Square(result.ky);

    result.domain_valid =
        kz_squared >= 0.0 &&
        xi3_argument > 0.0 &&
        xi5_argument > 0.0 &&
        eta2_argument > 0.0 &&
        eta4_argument > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_exact_domain";
        result.kz = NaN();
        result.xi3 = NaN();
        result.xi5 = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.b_over_A4 = config.b / result.A4;
        result.kz_normalized_against_n4 = NaN();
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.xi3 = 1.0 / std::sqrt(xi3_argument);
    result.xi5 = 1.0 / std::sqrt(xi5_argument);
    result.eta2 = 1.0 / std::sqrt(eta2_argument);
    result.eta4 = 1.0 / std::sqrt(eta4_argument);
    result.b_over_A4 = config.b / result.A4;

    const double denominator = Square(result.k1) - Square(result.k4);
    result.kz_normalized_against_n4 =
        denominator > 0.0 ? (Square(result.kz) - Square(result.k4)) / denominator : NaN();

    result.guided =
        Square(result.kz) > Square(result.k4) &&
        Square(result.kz) <= Square(result.k1) &&
        result.kz_normalized_against_n4 >= 0.0 &&
        result.kz_normalized_against_n4 <= 1.0;

    if (!result.guided) {
        result.status = "below_cutoff";
    }

    return result;
}

SingleGuideResult SolveSingleGuide(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveSingleGuideExact(config);
    }

    return SolveSingleGuideClosedForm(config);
}

}  // namespace marcatili
