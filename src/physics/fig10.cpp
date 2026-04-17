#include "marcatili/physics/fig10.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace marcatili {
namespace {

std::string FormatCompactNumber(double value) {
    std::ostringstream stream;
    stream << std::setprecision(6) << value;
    std::string text = stream.str();

    if (text.find('.') == std::string::npos) {
        return text;
    }

    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }

    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }

    return text;
}

void ValidateConfig(const Figure10Config& config) {
    if (config.c_over_a_min < 0.0 || config.c_over_a_max <= config.c_over_a_min) {
        throw std::runtime_error("c_over_a sweep bounds must define a non-negative interval.");
    }

    if (config.point_count < 2) {
        throw std::runtime_error("point_count must be at least 2.");
    }

    if (config.solver_models.empty()) {
        throw std::runtime_error("At least one solver model must be listed.");
    }

    if (config.curves.empty()) {
        throw std::runtime_error("At least one curve must be listed in a_over_A5_values.");
    }
}

}  // namespace

Figure10CurveSpec ParseFigure10CurveSpec(const std::string& value_text) {
    Figure10CurveSpec curve;
    curve.a_over_A5 = std::stod(value_text);
    curve.label = FormatCompactNumber(curve.a_over_A5);
    curve.curve_id = "a_over_A5=" + curve.label;
    return curve;
}

Figure10Result SolveFigure10(const Figure10Config& config) {
    ValidateConfig(config);

    Figure10Result result;
    result.config = config;
    result.status = "ok";

    const double step =
        (config.c_over_a_max - config.c_over_a_min) /
        static_cast<double>(config.point_count - 1);

    for (const auto& solver_model : config.solver_models) {
        for (const auto& curve : config.curves) {
            // Each Figure 10 family is a constant a/A_5 curve swept against c/a.
            // The actual coupling formula is centralized in SolveCouplerPoint.
            Figure10CurveSummary curve_summary;
            curve_summary.curve = curve;
            curve_summary.solver_model = solver_model;
            curve_summary.total_points = config.point_count;

            for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                const double c_over_a =
                    config.c_over_a_min + step * static_cast<double>(sample_index);

                CouplerPointConfig point_config;
                point_config.case_id = config.case_id + "_" + curve.curve_id;
                point_config.article_target = config.article_target;
                point_config.solver_model = solver_model;
                // Section IV cites Eq. (6) and Eq. (12) when introducing Fig. 10,
                // even though the modal label in the scan is OCR-ambiguous.
                // We keep that ambiguity explicit instead of silently "fixing" it.
                point_config.transverse_equation = CouplerTransverseEquation::kEq6;
                point_config.p = 1;
                point_config.a_over_A5 = curve.a_over_A5;
                point_config.c_over_a = c_over_a;

                const auto point = SolveCouplerPoint(point_config);

                if (sample_index == 0) {
                    // Recording the first-point root makes the report pedagogical:
                    // readers can see which transverse u = k_x A_5 / pi value was
                    // used for the whole family.
                    curve_summary.kx_A5_over_pi = point.kx_A5_over_pi;
                }

                if (point.domain_valid) {
                    ++curve_summary.valid_points;
                }

                result.samples.push_back({curve.curve_id, curve.label, sample_index, c_over_a, point});
            }

            result.curves.push_back(curve_summary);
        }
    }

    return result;
}

}  // namespace marcatili
