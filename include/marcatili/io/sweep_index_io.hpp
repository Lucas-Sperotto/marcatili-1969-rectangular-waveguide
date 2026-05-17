#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili::io {

struct SweepIndexModeSpec {
    marcatili::SingleGuideFamily family = marcatili::SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
};

struct SweepIndexBaseCase {
    double a = 0.0;
    double b = 0.0;
    double wavelength = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;
};

struct SweepIndexConfig {
    std::string case_id;
    std::string article_target;

    marcatili::SingleGuideSolverModel solver_model =
        marcatili::SingleGuideSolverModel::kExact;

    SweepIndexBaseCase base_case;

    std::string sweep_parameter;
    std::vector<double> sweep_values;
    std::vector<SweepIndexModeSpec> modes;
};

struct SweepIndexSample {
    double n4 = 0.0;
    marcatili::SingleGuideFamily family = marcatili::SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    double kz = 0.0;
    double ky = 0.0;
    double kx = 0.0;
};

SweepIndexConfig ParseSweepIndexConfig(const std::string& json_text);

std::string BuildSweepIndexCsvReport(const std::vector<SweepIndexSample>& samples);

}  // namespace marcatili::io
