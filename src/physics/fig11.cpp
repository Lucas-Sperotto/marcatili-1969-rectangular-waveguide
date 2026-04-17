#include "marcatili/physics/fig11.hpp"

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

void ValidateConfig(const Figure11Config& config) {
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

    if (config.index_ratios.empty()) {
        throw std::runtime_error("At least one index ratio must be listed in n1_over_n5_values.");
    }
}

}  // namespace

Figure11CurveSpec ParseFigure11CurveSpec(const std::string& value_text) {
    Figure11CurveSpec curve;
    curve.a_over_A5 = std::stod(value_text);
    curve.label = FormatCompactNumber(curve.a_over_A5);
    curve.curve_id = "a_over_A5=" + curve.label;
    return curve;
}

Figure11IndexRatioSpec ParseFigure11IndexRatioSpec(const std::string& value_text) {
    Figure11IndexRatioSpec ratio;
    ratio.n1_over_n5 = std::stod(value_text);
    // Eq. (20) is expressed in the code through r = (n5 / n1)^2, so the figure
    // parser converts the more article-friendly legend n1/n5 into that ratio once.
    ratio.index_ratio_squared = 1.0 / (ratio.n1_over_n5 * ratio.n1_over_n5);
    ratio.label = FormatCompactNumber(ratio.n1_over_n5);
    ratio.ratio_id = "n1_over_n5=" + ratio.label;
    return ratio;
}

Figure11Result SolveFigure11(const Figure11Config& config) {
    ValidateConfig(config);

    Figure11Result result;
    result.config = config;
    result.status = "ok";

    const double step =
        (config.c_over_a_max - config.c_over_a_min) /
        static_cast<double>(config.point_count - 1);

    for (const auto& solver_model : config.solver_models) {
        for (const auto& index_ratio : config.index_ratios) {
            for (const auto& curve : config.curves) {
                // Figure 11 adds one more sweep dimension relative to Figure 10:
                // each a/A_5 family is repeated for each legend value of n1/n5.
                Figure11CurveSummary curve_summary;
                curve_summary.curve = curve;
                curve_summary.index_ratio = index_ratio;
                curve_summary.solver_model = solver_model;
                curve_summary.total_points = config.point_count;

                for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                    const double c_over_a =
                        config.c_over_a_min + step * static_cast<double>(sample_index);

                    CouplerPointConfig point_config;
                    point_config.case_id =
                        config.case_id + "_" + index_ratio.ratio_id + "_" + curve.curve_id;
                    point_config.article_target = config.article_target;
                    point_config.solver_model = solver_model;
                    // Section IV states that Fig. 11 uses Eq. (20) for the exact transverse root.
                    point_config.transverse_equation = CouplerTransverseEquation::kEq20;
                    point_config.p = 1;
                    point_config.a_over_A5 = curve.a_over_A5;
                    point_config.c_over_a = c_over_a;
                    point_config.index_ratio_squared = index_ratio.index_ratio_squared;

                    const auto point = SolveCouplerPoint(point_config);

                    if (sample_index == 0) {
                        // As in Fig. 10, this makes the link between the family label
                        // and the reused single-guide transverse root visible.
                        curve_summary.kx_A5_over_pi = point.kx_A5_over_pi;
                    }

                    if (point.domain_valid) {
                        ++curve_summary.valid_points;
                    }

                    result.samples.push_back(
                        {index_ratio.ratio_id,
                         index_ratio.label,
                         curve.curve_id,
                         curve.label,
                         sample_index,
                         c_over_a,
                         point}
                    );
                }

                result.curves.push_back(curve_summary);
            }
        }
    }

    return result;
}

}  // namespace marcatili
