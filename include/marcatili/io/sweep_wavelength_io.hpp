#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili::io {

struct SweepWavelengthModeSpec {
    marcatili::SingleGuideFamily family = marcatili::SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
};

struct SweepWavelengthConfig {
    std::string case_id;
    std::string article_target;

    marcatili::SingleGuideSolverModel solver_model =
        marcatili::SingleGuideSolverModel::kExact;

    double a = 0.0;
    double b = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;

    double min_wavelength = 0.0;
    double max_wavelength = 0.0;
    int point_count = 0;

    std::vector<SweepWavelengthModeSpec> modes;
};

struct SweepWavelengthSample {
    double wavelength = 0.0;
    marcatili::SingleGuideFamily family = marcatili::SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    double kz = 0.0;
    double ky = 0.0;
    double kx = 0.0;
};

SweepWavelengthConfig ParseSweepWavelengthConfig(const std::string& json_text);

std::string BuildSweepWavelengthCsvReport(
    const std::vector<SweepWavelengthSample>& samples
);

}  // namespace marcatili::io
