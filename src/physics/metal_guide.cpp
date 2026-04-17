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

SingleGuideResult SolveMetalGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    if (config.family == SingleGuideFamily::kEy) {
        const double x_denominator = 1.0 + (result.A3 + result.A5) / (kPi * config.a);
        const double y_denominator =
            1.0 + (Square(config.n4) * result.A4) / (kPi * Square(config.n1) * config.b);

        result.kx = (config.p * kPi / config.a) / x_denominator;
        // For E_y, the metalized upper boundary is modeled as a fixed pi/2 phase term.
        result.ky = ((static_cast<double>(config.q) - 0.5) * kPi / config.b) / y_denominator;
        result.equations_used =
            "Fig. 8 metal-boundary approximation for E_y derived from (6), (7), (12), (13) "
            "with n3=n5=n4 and the top interface replaced by a PEC phase term pi/2";
    } else {
        const double x_denominator =
            1.0 +
            (Square(config.n4) * (result.A3 + result.A5)) /
                (kPi * Square(config.n1) * config.a);
        // For E_x, the tangential electric field vanishes on the metal wall, so the
        // top-interface arctangent term is removed while the dielectric bottom term remains.
        const double y_denominator = 1.0 + result.A4 / (kPi * config.b);

        result.kx = (config.p * kPi / config.a) / x_denominator;
        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;
        result.equations_used =
            "Fig. 8 metal-boundary approximation for E_x inferred from (20), (21), (22), (23) "
            "with n3=n5=n4, a PEC top boundary and only the lower dielectric term kept in y";
    }

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

SingleGuideResult SolveMetalGuideExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    std::function<double(double)> fx;
    std::function<double(double)> fy;

    if (config.family == SingleGuideFamily::kEy) {
        fx = [&](double kx_value) {
            const double xi = PenetrationDepth(result.A4, kx_value);
            return kx_value * config.a + 2.0 * std::atan(kx_value * xi) -
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
            return ky_value * config.b + std::atan(ky_value * eta4) -
                   static_cast<double>(config.q) * kPi;
        };
    }

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

    result.kx = math::SolveRootByBisection(fx, lower, x_upper);
    result.ky = math::SolveRootByBisection(fy, lower, y_upper);
    if (config.family == SingleGuideFamily::kEy) {
        result.equations_used =
            "Fig. 8 metal-boundary exact model for E_y derived from (6), (7) and Appendix A.1, "
            "using n3=n5=n4 and a PEC phase condition on the top interface";
    } else {
        result.equations_used =
            "Fig. 8 metal-boundary exact model for E_x inferred from (20), (21), (18), (19), "
            "using n3=n5=n4, a PEC top boundary and a single lower dielectric phase term in y";
    }

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

SingleGuideResult SolveMetalGuide(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveMetalGuideExact(config);
    }

    return SolveMetalGuideClosedForm(config);
}

}  // namespace marcatili
