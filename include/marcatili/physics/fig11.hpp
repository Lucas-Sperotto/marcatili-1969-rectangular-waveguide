#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/coupler.hpp"

namespace marcatili {

struct Figure11CurveSpec {
    double a_over_A5 = 0.0;
    std::string curve_id;
    std::string label;
};

struct Figure11IndexRatioSpec {
    double n1_over_n5 = 0.0;
    double index_ratio_squared = 0.0;
    std::string ratio_id;
    std::string label;
};

struct Figure11Config {
    std::string case_id;
    std::string article_target;
    std::string ocr_note;
    std::string csv_output_path;
    double c_over_a_min = 0.0;
    double c_over_a_max = 0.0;
    int point_count = 0;
    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Figure11CurveSpec> curves;
    std::vector<Figure11IndexRatioSpec> index_ratios;
};

struct Figure11CurveSummary {
    Figure11CurveSpec curve;
    Figure11IndexRatioSpec index_ratio;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kExact;
    int total_points = 0;
    int valid_points = 0;
    double kx_A5_over_pi = 0.0;
};

struct Figure11Sample {
    std::string ratio_id;
    std::string ratio_label;
    std::string curve_id;
    std::string curve_label;
    int sample_index = 0;
    double c_over_a = 0.0;
    CouplerPointResult point;
};

struct Figure11Result {
    Figure11Config config;
    std::string status;
    std::vector<Figure11CurveSummary> curves;
    std::vector<Figure11Sample> samples;
};

Figure11CurveSpec ParseFigure11CurveSpec(const std::string& value_text);
Figure11IndexRatioSpec ParseFigure11IndexRatioSpec(const std::string& value_text);
Figure11Result SolveFigure11(const Figure11Config& config);

}  // namespace marcatili
