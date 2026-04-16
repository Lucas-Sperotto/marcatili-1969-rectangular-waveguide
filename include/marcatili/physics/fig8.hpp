#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

struct Figure8ModeSpec {
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    std::string curve_id;
};

struct Figure8Config {
    std::string case_id;
    std::string article_target;
    std::string ocr_note;
    std::string csv_output_path;
    double wavelength = 0.0;
    double a_over_b = 0.0;
    double n1 = 0.0;
    double n4 = 0.0;
    double a_over_A_min = 0.0;
    double a_over_A_max = 0.0;
    int point_count = 0;
    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Figure8ModeSpec> modes;
};

struct Figure8CurveSummary {
    Figure8ModeSpec mode;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    int total_points = 0;
    int guided_points = 0;
    int valid_points = 0;
};

struct Figure8Sample {
    std::string curve_id;
    int sample_index = 0;
    double a_over_A = 0.0;
    SingleGuideResult point;
};

struct Figure8Result {
    Figure8Config config;
    std::string status;
    std::vector<Figure8CurveSummary> curves;
    std::vector<Figure8Sample> samples;
};

Figure8ModeSpec ParseFigure8ModeSpec(const std::string& mode_text);
Figure8Result SolveFigure8(const Figure8Config& config);

}  // namespace marcatili
