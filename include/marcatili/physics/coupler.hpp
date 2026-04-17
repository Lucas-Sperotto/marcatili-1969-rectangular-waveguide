#pragma once

#include <string>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

enum class CouplerTransverseEquation {
    kEq6,
    kEq20
};

struct CouplerPointConfig {
    std::string case_id;
    std::string article_target;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    // Eq. (6) is used for the Fig. 10 repository baseline; Eq. (20) is used for Fig. 11.
    CouplerTransverseEquation transverse_equation = CouplerTransverseEquation::kEq6;
    // Marcatili uses p as the horizontal mode index in the transverse characteristic equation.
    int p = 1;
    // Normalized electrical width a / A_5 = (2 a / lambda) sqrt(n_1^2 - n_5^2).
    double a_over_A5 = 0.0;
    // Horizontal separation normalized by the guide width, matching the x-axis of Figs. 10 and 11.
    double c_over_a = 0.0;
    // Symmetric-limit helper for Eq. (20) and Eq. (22): r = (n_5 / n_1)^2.
    double index_ratio_squared = 0.0;
};

struct CouplerPointResult {
    CouplerPointConfig config;
    bool domain_valid = false;
    std::string status;
    std::string equations_used;
    double a_over_A5 = 0.0;
    double c_over_A5 = 0.0;
    double kx_A5_over_pi = 0.0;
    double sqrt_one_minus_kx_A5_over_pi_squared = 0.0;
    double normalized_coupling = 0.0;
};

std::string ToString(CouplerTransverseEquation equation);
CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config);

}  // namespace marcatili
