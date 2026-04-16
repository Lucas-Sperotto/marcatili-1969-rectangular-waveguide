#include "marcatili/physics/metal_guide.hpp"

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
    if (config.family != SingleGuideFamily::kEy) {
        throw std::runtime_error("The current Fig. 8 metal-boundary model only supports E_y modes.");
    }

    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.a <= 0.0 || config.b <= 0.0) {
        throw std::runtime_error("a and b must be positive.");
    }

    if (config.p <= 0 || config.q <= 0) {
        throw std::runtime_error("mode indices p and q must be positive integers.");
    }

    if (config.n1 <= config.n4) {
        throw std::runtime_error("The Fig. 8 metal-boundary model requires n1 > n4.");
    }
}

SingleGuideResult BuildBaseResult(const SingleGuideConfig& config) {
    SingleGuideResult result;
    result.config = config;
    result.status = "ok";
    result.k0 = 2.0 * kPi / config.wavelength;
    result.k1 = result.k0 * config.n1;
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

SingleGuideResult SolveMetalGuideEyClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    const double x_denominator = 1.0 + (result.A3 + result.A5) / (kPi * config.a);
    const double y_denominator =
        1.0 + (Square(config.n4) * result.A4) / (kPi * Square(config.n1) * config.b);

    result.kx = (config.p * kPi / config.a) / x_denominator;
    // The metalized upper boundary is modeled as a PEC phase shift of pi/2 in the y equation.
    result.ky = ((static_cast<double>(config.q) - 0.5) * kPi / config.b) / y_denominator;
    result.equations_used =
        "Fig. 8 metal-boundary approximation derived from (6), (7), (12), (13) "
        "with n3=n5=n4 and the top interface replaced by a PEC phase term pi/2";

    result.approximation_checks.kx_a3_over_pi_squared = Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared = Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared = NaN();
    result.approximation_checks.ky_a4_over_pi_squared = Square(result.ky * result.A4 / kPi);

    const double kz_squared = Square(result.k1) - Square(result.kx) - Square(result.ky);
    const double xi_argument = 1.0 - result.approximation_checks.kx_a3_over_pi_squared;
    const double eta4_argument = 1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    result.domain_valid = kz_squared >= 0.0 && xi_argument > 0.0 && eta4_argument > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_closed_form_domain";
        result.kz = NaN();
        result.xi3 = NaN();
        result.xi5 = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.kz_normalized_against_n4 = NaN();
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

SingleGuideResult SolveMetalGuideEyExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    const auto fx = [&](double kx_value) {
        const double xi = PenetrationDepth(result.A4, kx_value);
        return kx_value * config.a + 2.0 * std::atan(kx_value * xi) -
               static_cast<double>(config.p) * kPi;
    };

    const auto fy = [&](double ky_value) {
        const double eta4 = PenetrationDepth(result.A4, ky_value);
        return ky_value * config.b +
               std::atan((Square(config.n4) / Square(config.n1)) * ky_value * eta4) -
               (static_cast<double>(config.q) - 0.5) * kPi;
    };

    const double x_upper = SafeUpperBound(kPi / result.A3, kPi / result.A5);
    const double y_upper = kPi / result.A4 * (1.0 - 1e-10);
    const double lower = 1e-12;

    if (!(fx(lower) < 0.0 && fx(x_upper) > 0.0) ||
        !(fy(lower) < 0.0 && fy(y_upper) > 0.0)) {
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
        result.kz_normalized_against_n4 = NaN();
        return result;
    }

    result.kx = RootSolveByBisection(fx, lower, x_upper);
    result.ky = RootSolveByBisection(fy, lower, y_upper);
    result.equations_used =
        "Fig. 8 metal-boundary exact model derived from (6), (7) and Appendix A.1, "
        "using n3=n5=n4 and a PEC phase condition on the top interface";

    result.approximation_checks.kx_a3_over_pi_squared = Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared = Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared = NaN();
    result.approximation_checks.ky_a4_over_pi_squared = Square(result.ky * result.A4 / kPi);

    const double kz_squared = Square(result.k1) - Square(result.kx) - Square(result.ky);
    const double xi_argument = Square(kPi / result.A4) - Square(result.kx);
    const double eta4_argument = Square(kPi / result.A4) - Square(result.ky);

    result.domain_valid = kz_squared >= 0.0 && xi_argument > 0.0 && eta4_argument > 0.0;

    if (!result.domain_valid) {
        result.status = "outside_exact_domain";
        result.kz = NaN();
        result.xi3 = NaN();
        result.xi5 = NaN();
        result.eta2 = NaN();
        result.eta4 = NaN();
        result.kz_normalized_against_n4 = NaN();
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

SingleGuideResult SolveMetalGuideEy(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveMetalGuideEyExact(config);
    }

    return SolveMetalGuideEyClosedForm(config);
}

}  // namespace marcatili
