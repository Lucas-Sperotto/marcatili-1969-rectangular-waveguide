#pragma once

#include <string>

namespace marcatili {

enum class SingleGuideFamily {
    kEy,
    kEx
};

enum class SingleGuideSolverModel {
    kClosedForm,
    kExact
};

struct SingleGuideConfig {
    std::string case_id;
    std::string article_target;
    std::string csv_output_path;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    double wavelength = 0.0;
    double a = 0.0;
    double b = 0.0;
    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;
};

struct SingleGuideApproximationChecks {
    double kx_a3_over_pi_squared = 0.0;
    double kx_a5_over_pi_squared = 0.0;
    double ky_a2_over_pi_squared = 0.0;
    double ky_a4_over_pi_squared = 0.0;
};

struct SingleGuideResult {
    SingleGuideConfig config;
    bool domain_valid = false;
    bool guided = false;
    std::string status;
    std::string equations_used;
    double k0 = 0.0;
    double k1 = 0.0;
    double k2 = 0.0;
    double k3 = 0.0;
    double k4 = 0.0;
    double k5 = 0.0;
    double A2 = 0.0;
    double A3 = 0.0;
    double A4 = 0.0;
    double A5 = 0.0;
    double kx = 0.0;
    double ky = 0.0;
    double kz = 0.0;
    double xi3 = 0.0;
    double xi5 = 0.0;
    double eta2 = 0.0;
    double eta4 = 0.0;
    double b_over_A4 = 0.0;
    double kz_normalized_against_n4 = 0.0;
    SingleGuideApproximationChecks approximation_checks;
};

std::string ToString(SingleGuideFamily family);
SingleGuideFamily ParseSingleGuideFamily(const std::string& family_text);
std::string ToString(SingleGuideSolverModel solver_model);
SingleGuideSolverModel ParseSingleGuideSolverModel(const std::string& solver_model_text);
SingleGuideResult SolveSingleGuideClosedForm(const SingleGuideConfig& config);
SingleGuideResult SolveSingleGuideExact(const SingleGuideConfig& config);
SingleGuideResult SolveSingleGuide(const SingleGuideConfig& config);

}  // namespace marcatili
