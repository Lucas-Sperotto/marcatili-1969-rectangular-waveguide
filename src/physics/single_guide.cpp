#include "marcatili/physics/single_guide.hpp"

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

std::string ClassifyStatus(const std::string& status) {
    if (status == "ok") {
        return "solution";
    }

    if (status == "below_cutoff") {
        return "physical_limit";
    }

    return "domain_limit";
}

std::string NormalizeToken(const std::string& text) {
    std::string normalized;
    normalized.reserve(text.size());

    for (unsigned char ch : text) {
        if (std::isalnum(ch)) {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }

    return normalized;
}

void ResetDependentOutputs(SingleGuideResult& result) {
    result.kz = NaN();
    result.xi3 = NaN();
    result.xi5 = NaN();
    result.eta2 = NaN();
    result.eta4 = NaN();
    result.kz_normalized_against_n4 = NaN();
    result.guided = false;
}

void SetUnavailableResult(
    SingleGuideResult& result,
    const char* status,
    bool domain_valid
) {
    result.status = status;
    result.status_class = ClassifyStatus(result.status);
    ResetDependentOutputs(result);
    result.domain_valid = domain_valid;
}

void ValidateConfig(const SingleGuideConfig& config) {
    if (!IsFiniteNumber(config.wavelength) || config.wavelength <= 0.0) {
        throw std::invalid_argument(
            "SolveSingleGuide: wavelength must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.a) || !IsFiniteNumber(config.b) ||
        config.a <= 0.0 || config.b <= 0.0) {
        throw std::invalid_argument(
            "SolveSingleGuide: a and b must be finite positive values."
        );
    }

    if (config.p <= 0 || config.q <= 0) {
        throw std::invalid_argument(
            "SolveSingleGuide: mode indices p and q must be positive integers."
        );
    }

    if (!IsFiniteNumber(config.n1) || config.n1 <= 0.0 ||
        !IsFiniteNumber(config.n2) || config.n2 <= 0.0 ||
        !IsFiniteNumber(config.n3) || config.n3 <= 0.0 ||
        !IsFiniteNumber(config.n4) || config.n4 <= 0.0 ||
        !IsFiniteNumber(config.n5) || config.n5 <= 0.0) {
        throw std::invalid_argument(
            "SolveSingleGuide: refractive indices n1..n5 must be finite positive values."
        );
    }

    const double external_max =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));

    if (!(config.n1 > external_max)) {
        throw std::invalid_argument(
            "SolveSingleGuide: requires n1 > n2, n3, n4 and n5."
        );
    }
}

SingleGuideResult BuildBaseResult(const SingleGuideConfig& config) {
    SingleGuideResult result;
    result.config = config;
    result.status = "ok";
    result.status_class = ClassifyStatus(result.status);

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

    result.b_over_A4 = config.b / result.A4;
    result.critical_external_index =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));
    result.critical_external_wave_number = result.k0 * result.critical_external_index;

    return result;
}

void FinalizeResult(SingleGuideResult& result) {
    const double denominator = Square(result.k1) - Square(result.k4);

    result.kz_normalized_against_n4 =
        (denominator > 0.0 && IsFiniteNumber(result.kz))
            ? (Square(result.kz) - Square(result.k4)) / denominator
            : NaN();

    // A normalização por n4 é usada porque a Fig. 6 toma A4 e o clad inferior
    // como referência de reporte. Isso é uma convenção de apresentação.
    result.guided =
        IsFiniteNumber(result.kz) &&
        Square(result.kz) > Square(result.critical_external_wave_number) &&
        Square(result.kz) <= Square(result.k1) &&
        IsFiniteNumber(result.kz_normalized_against_n4) &&
        result.kz_normalized_against_n4 >= 0.0 &&
        result.kz_normalized_against_n4 <= 1.0;

    result.domain_valid = true;

    if (!result.guided) {
        result.status = "below_cutoff";
    }

    result.status_class = ClassifyStatus(result.status);
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

double SolveExactRoot(
    const std::function<double(double)>& function,
    double upper_bound
) {
    if (!BracketsRoot(function, kRootLowerBound, upper_bound)) {
        throw std::runtime_error(
            "SolveSingleGuideExact: no guided root found in the allowed transverse interval."
        );
    }

    return math::SolveRootByBisection(function, kRootLowerBound, upper_bound);
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

RootSearchResult SolveExactRootWithStatus(
    const std::function<double(double)>& function,
    double upper_bound
) {
    const double f_lower = function(kRootLowerBound);
    const double f_upper = function(upper_bound);

    if (!IsFiniteNumber(f_lower) || !IsFiniteNumber(f_upper) || !(f_lower < 0.0)) {
        return {RootSearchStatus::kOutsideDomain, NaN()};
    }

    if (f_upper <= 0.0) {
        return {RootSearchStatus::kBelowCutoff, NaN()};
    }

    if (!BracketsRoot(function, kRootLowerBound, upper_bound)) {
        return {RootSearchStatus::kOutsideDomain, NaN()};
    }

    return {
        RootSearchStatus::kFound,
        SolveExactRoot(function, upper_bound)
    };
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
    const std::string normalized = NormalizeToken(family_text);

    if (normalized == "ey") {
        return SingleGuideFamily::kEy;
    }

    if (normalized == "ex") {
        return SingleGuideFamily::kEx;
    }

    throw std::invalid_argument(
        "ParseSingleGuideFamily: supported values are E_y, Ey, E^y, E_x, Ex and E^x."
    );
}

SingleGuideSolverModel ParseSingleGuideSolverModel(const std::string& solver_model_text) {
    const std::string normalized = NormalizeToken(solver_model_text);

    if (normalized == "closedform" || normalized == "approx" || normalized == "approximate") {
        return SingleGuideSolverModel::kClosedForm;
    }

    if (normalized == "exact" || normalized == "transcendental") {
        return SingleGuideSolverModel::kExact;
    }

    throw std::invalid_argument(
        "ParseSingleGuideSolverModel: supported values are closed_form and exact."
    );
}

SingleGuideResult SolveSingleGuideClosedForm(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    if (config.family == SingleGuideFamily::kEy) {
        // Família E_y: aproximações algébricas associadas às Eqs. (12) e (13).
        const double x_denominator =
            1.0 + (result.A3 + result.A5) / (kPi * config.a);

        const double y_denominator =
            1.0 +
            (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
                (kPi * Square(config.n1) * config.b);

        result.kx = (static_cast<double>(config.p) * kPi / config.a) / x_denominator;
        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;
        result.equations_used = "(10), (12), (13), (14), (15), (16)";
    } else {
        // Família E_x: aproximações algébricas associadas às Eqs. (22) e (23).
        const double x_denominator =
            1.0 +
            (Square(config.n3) * result.A3 + Square(config.n5) * result.A5) /
                (kPi * Square(config.n1) * config.a);

        const double y_denominator =
            1.0 + (result.A2 + result.A4) / (kPi * config.b);

        result.kx = (static_cast<double>(config.p) * kPi / config.a) / x_denominator;
        result.ky = (static_cast<double>(config.q) * kPi / config.b) / y_denominator;
        result.equations_used = "(10), (22), (23), (24), (25), (26)";
    }

    result.approximation_checks.kx_a3_over_pi_squared =
        Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared =
        Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared =
        Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);

    const double kz_squared =
        Square(result.k1) - Square(result.kx) - Square(result.ky);

    const double xi3_argument =
        1.0 - result.approximation_checks.kx_a3_over_pi_squared;
    const double xi5_argument =
        1.0 - result.approximation_checks.kx_a5_over_pi_squared;
    const double eta2_argument =
        1.0 - result.approximation_checks.ky_a2_over_pi_squared;
    const double eta4_argument =
        1.0 - result.approximation_checks.ky_a4_over_pi_squared;

    const bool domain_valid =
        IsFiniteNumber(kz_squared) && kz_squared >= 0.0 &&
        IsFiniteNumber(xi3_argument) && xi3_argument > 0.0 &&
        IsFiniteNumber(xi5_argument) && xi5_argument > 0.0 &&
        IsFiniteNumber(eta2_argument) && eta2_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!domain_valid) {
        SetUnavailableResult(result, "outside_closed_form_domain", false);
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.xi3 = (result.A3 / kPi) / std::sqrt(xi3_argument);
    result.xi5 = (result.A5 / kPi) / std::sqrt(xi5_argument);
    result.eta2 = (result.A2 / kPi) / std::sqrt(eta2_argument);
    result.eta4 = (result.A4 / kPi) / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

SingleGuideResult SolveSingleGuideExact(const SingleGuideConfig& config) {
    ValidateConfig(config);

    SingleGuideResult result = BuildBaseResult(config);

    RootSearchResult root_x;
    RootSearchResult root_y;

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

        root_x = SolveExactRootWithStatus(
            fx,
            SafeUpperBound(kPi / result.A3, kPi / result.A5)
        );
        root_y = SolveExactRootWithStatus(
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

        root_x = SolveExactRootWithStatus(
            fx,
            SafeUpperBound(kPi / result.A3, kPi / result.A5)
        );
        root_y = SolveExactRootWithStatus(
            fy,
            SafeUpperBound(kPi / result.A2, kPi / result.A4)
        );
        result.equations_used = "(20), (21), (18), (19), (10)";
    }

    if (root_x.status == RootSearchStatus::kOutsideDomain ||
        root_y.status == RootSearchStatus::kOutsideDomain) {
        result.kx = NaN();
        result.ky = NaN();
        SetUnavailableResult(result, "outside_exact_domain", false);
        return result;
    }

    if (root_x.status == RootSearchStatus::kBelowCutoff ||
        root_y.status == RootSearchStatus::kBelowCutoff) {
        result.kx = NaN();
        result.ky = NaN();
        SetUnavailableResult(result, "below_cutoff", true);
        return result;
    }

    result.kx = root_x.root;
    result.ky = root_y.root;

    result.approximation_checks.kx_a3_over_pi_squared =
        Square(result.kx * result.A3 / kPi);
    result.approximation_checks.kx_a5_over_pi_squared =
        Square(result.kx * result.A5 / kPi);
    result.approximation_checks.ky_a2_over_pi_squared =
        Square(result.ky * result.A2 / kPi);
    result.approximation_checks.ky_a4_over_pi_squared =
        Square(result.ky * result.A4 / kPi);

    // Uma vez encontrados kx e ky, kz vem da Eq. (10), e xi/eta são os
    // comprimentos de decaimento evanescente nos meios externos.
    const double kz_squared =
        Square(result.k1) - Square(result.kx) - Square(result.ky);

    const double xi3_argument =
        Square(kPi / result.A3) - Square(result.kx);
    const double xi5_argument =
        Square(kPi / result.A5) - Square(result.kx);
    const double eta2_argument =
        Square(kPi / result.A2) - Square(result.ky);
    const double eta4_argument =
        Square(kPi / result.A4) - Square(result.ky);

    const bool decay_domain_valid =
        IsFiniteNumber(xi3_argument) && xi3_argument > 0.0 &&
        IsFiniteNumber(xi5_argument) && xi5_argument > 0.0 &&
        IsFiniteNumber(eta2_argument) && eta2_argument > 0.0 &&
        IsFiniteNumber(eta4_argument) && eta4_argument > 0.0;

    if (!(IsFiniteNumber(kz_squared)) || !decay_domain_valid) {
        SetUnavailableResult(result, "outside_exact_domain", false);
        return result;
    }

    if (kz_squared < 0.0) {
        SetUnavailableResult(result, "below_cutoff", true);
        return result;
    }

    result.kz = std::sqrt(kz_squared);
    result.xi3 = 1.0 / std::sqrt(xi3_argument);
    result.xi5 = 1.0 / std::sqrt(xi5_argument);
    result.eta2 = 1.0 / std::sqrt(eta2_argument);
    result.eta4 = 1.0 / std::sqrt(eta4_argument);

    FinalizeResult(result);
    return result;
}

SingleGuideResult SolveSingleGuide(const SingleGuideConfig& config) {
    if (config.solver_model == SingleGuideSolverModel::kExact) {
        return SolveSingleGuideExact(config);
    }

    return SolveSingleGuideClosedForm(config);
}

}  // namespace marcatili
