#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/coupler.hpp"

namespace marcatili {

struct Figure10CurveSpec {
    double a_over_A5 = 0.0;
    std::string curve_id;
    std::string label;
};

struct Figure10Config {
    std::string case_id;
    std::string article_target;
    std::string ocr_note;
    std::string csv_output_path;
    double c_over_a_min = 0.0;
    double c_over_a_max = 0.0;
    int point_count = 0;
    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Figure10CurveSpec> curves;
};

struct Figure10CurveSummary {
    Figure10CurveSpec curve;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    int total_points = 0;
    int valid_points = 0;
    double kx_A5_over_pi = 0.0;
};

struct Figure10Sample {
    std::string curve_id;
    std::string curve_label;
    int sample_index = 0;
    double c_over_a = 0.0;
    CouplerPointResult point;
};

struct Figure10Result {
    Figure10Config config;
    std::string status;
    std::vector<Figure10CurveSummary> curves;
    std::vector<Figure10Sample> samples;
};

Figure10CurveSpec ParseFigure10CurveSpec(const std::string& value_text);
Figure10Result SolveFigure10(const Figure10Config& config);

}  // namespace marcatili
