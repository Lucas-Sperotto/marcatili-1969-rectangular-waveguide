#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili::io {

/**
 * @brief Mode specification for the fixed-b/A4 aspect-ratio sweep.
 */
struct SweepAspectModeSpec {
    marcatili::SingleGuideFamily family = marcatili::SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
};

/**
 * @brief Input configuration for reproduce_sweep_aspect.
 */
struct SweepAspectConfig {
    std::string case_id;
    std::string article_target;

    marcatili::SingleGuideSolverModel solver_model =
        marcatili::SingleGuideSolverModel::kExact;

    double wavelength = 0.0;
    double b_over_A4 = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;

    double a_over_b_min = 0.0;
    double a_over_b_max = 0.0;
    int point_count = 0;

    std::vector<SweepAspectModeSpec> modes;
};

/**
 * @brief One CSV row from the aspect-ratio sweep.
 */
struct SweepAspectSample {
    double a_over_b = 0.0;
    marcatili::SingleGuideFamily family = marcatili::SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    double kz = 0.0;
    double ky = 0.0;
    double kx = 0.0;
};

/**
 * @brief Parse reproduce_sweep_aspect input JSON.
 */
SweepAspectConfig ParseSweepAspectConfig(const std::string& json_text);

/**
 * @brief Build stdout CSV for reproduce_sweep_aspect.
 */
std::string BuildSweepAspectCsvReport(const std::vector<SweepAspectSample>& samples);

}  // namespace marcatili::io
