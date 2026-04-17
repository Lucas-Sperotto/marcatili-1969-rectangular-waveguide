#include "marcatili/physics/fig11.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace marcatili {
namespace {

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

std::string FormatCompactNumber(double value) {
    std::ostringstream stream;
    stream << std::setprecision(6) << std::defaultfloat << value;
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

Figure11CurveSpec NormalizeCurveSpec(const Figure11CurveSpec& curve) {
    if (!IsFiniteNumber(curve.a_over_A5) || curve.a_over_A5 <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure11: each curve must provide a finite positive a_over_A5."
        );
    }

    Figure11CurveSpec normalized = curve;

    if (normalized.label.empty()) {
        normalized.label = FormatCompactNumber(normalized.a_over_A5);
    }

    if (normalized.curve_id.empty()) {
        normalized.curve_id = "a_over_A5=" + normalized.label;
    }

    return normalized;
}

Figure11IndexRatioSpec NormalizeIndexRatioSpec(const Figure11IndexRatioSpec& ratio) {
    Figure11IndexRatioSpec normalized = ratio;

    const bool has_n1_over_n5 =
        IsFiniteNumber(normalized.n1_over_n5) && normalized.n1_over_n5 > 0.0;

    const bool has_index_ratio_squared =
        IsFiniteNumber(normalized.index_ratio_squared) &&
        normalized.index_ratio_squared > 0.0 &&
        normalized.index_ratio_squared < 1.0;

    if (!has_n1_over_n5 && !has_index_ratio_squared) {
        throw std::invalid_argument(
            "SolveFigure11: each index ratio must provide either n1_over_n5 > 0 "
            "or 0 < index_ratio_squared < 1."
        );
    }

    if (has_n1_over_n5 && !has_index_ratio_squared) {
        normalized.index_ratio_squared =
            1.0 / (normalized.n1_over_n5 * normalized.n1_over_n5);
    } else if (!has_n1_over_n5 && has_index_ratio_squared) {
        normalized.n1_over_n5 =
            1.0 / std::sqrt(normalized.index_ratio_squared);
    } else {
        const double reconstructed =
            1.0 / (normalized.n1_over_n5 * normalized.n1_over_n5);

        if (std::abs(reconstructed - normalized.index_ratio_squared) > 1e-12) {
            throw std::invalid_argument(
                "SolveFigure11: inconsistent n1_over_n5 and index_ratio_squared."
            );
        }
    }

    if (!(normalized.index_ratio_squared > 0.0 &&
          normalized.index_ratio_squared < 1.0)) {
        throw std::invalid_argument(
            "SolveFigure11: index_ratio_squared must satisfy 0 < r < 1."
        );
    }

    if (!(normalized.n1_over_n5 > 1.0)) {
        throw std::invalid_argument(
            "SolveFigure11: n1_over_n5 must be greater than 1."
        );
    }

    if (normalized.label.empty()) {
        normalized.label = FormatCompactNumber(normalized.n1_over_n5);
    }

    if (normalized.ratio_id.empty()) {
        normalized.ratio_id = "n1_over_n5=" + normalized.label;
    }

    return normalized;
}

void ValidateConfig(const Figure11Config& config) {
    if (!IsFiniteNumber(config.c_over_a_min) || !IsFiniteNumber(config.c_over_a_max)) {
        throw std::invalid_argument(
            "SolveFigure11: c_over_a sweep bounds must be finite."
        );
    }

    if (config.c_over_a_min < 0.0 || config.c_over_a_max <= config.c_over_a_min) {
        throw std::invalid_argument(
            "SolveFigure11: c_over_a sweep bounds must define a non-negative interval."
        );
    }

    if (config.point_count < 2) {
        throw std::invalid_argument(
            "SolveFigure11: point_count must be at least 2."
        );
    }

    if (config.solver_models.empty()) {
        throw std::invalid_argument(
            "SolveFigure11: at least one solver model must be listed."
        );
    }

    if (config.curves.empty()) {
        throw std::invalid_argument(
            "SolveFigure11: at least one curve must be listed."
        );
    }

    if (config.index_ratios.empty()) {
        throw std::invalid_argument(
            "SolveFigure11: at least one index ratio must be listed."
        );
    }

    for (const auto& curve : config.curves) {
        NormalizeCurveSpec(curve);
    }

    for (const auto& ratio : config.index_ratios) {
        NormalizeIndexRatioSpec(ratio);
    }
}

CouplerPointResult SolveFigure11Point(
    const Figure11Config& config,
    const Figure11IndexRatioSpec& index_ratio,
    const Figure11CurveSpec& curve,
    SingleGuideSolverModel solver_model,
    double c_over_a
) {
    CouplerPointConfig point_config;
    point_config.case_id =
        config.case_id + "_" + index_ratio.ratio_id + "_" + curve.curve_id + "_" +
        ToString(solver_model);
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = solver_model;

    // A base operacional da Fig. 11 usa Eq. (20) para a raiz transversal.
    point_config.transverse_equation = CouplerTransverseEquation::kEq20;
    point_config.p = 1;
    point_config.a_over_A5 = curve.a_over_A5;
    point_config.c_over_a = c_over_a;
    point_config.index_ratio_squared = index_ratio.index_ratio_squared;

    return SolveCouplerPoint(point_config);
}

}  // namespace

Figure11CurveSpec ParseFigure11CurveSpec(const std::string& value_text) {
    Figure11CurveSpec curve;

    try {
        curve.a_over_A5 = std::stod(value_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure11CurveSpec: invalid curve specification '" + value_text +
            "'. Expected a numeric value for a_over_A5."
        );
    }

    curve = NormalizeCurveSpec(curve);
    return curve;
}

Figure11IndexRatioSpec ParseFigure11IndexRatioSpec(const std::string& value_text) {
    Figure11IndexRatioSpec ratio;

    try {
        ratio.n1_over_n5 = std::stod(value_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure11IndexRatioSpec: invalid index-ratio specification '" + value_text +
            "'. Expected a numeric value for n1_over_n5."
        );
    }

    ratio = NormalizeIndexRatioSpec(ratio);
    return ratio;
}

Figure11Result SolveFigure11(const Figure11Config& config) {
    ValidateConfig(config);

    Figure11Result result;
    result.config = config;
    result.status = "ok";

    std::vector<Figure11CurveSpec> normalized_curves;
    normalized_curves.reserve(config.curves.size());
    for (const auto& curve : config.curves) {
        normalized_curves.push_back(NormalizeCurveSpec(curve));
    }

    std::vector<Figure11IndexRatioSpec> normalized_ratios;
    normalized_ratios.reserve(config.index_ratios.size());
    for (const auto& ratio : config.index_ratios) {
        normalized_ratios.push_back(NormalizeIndexRatioSpec(ratio));
    }

    const double step =
        (config.c_over_a_max - config.c_over_a_min) /
        static_cast<double>(config.point_count - 1);

    for (const auto& solver_model : config.solver_models) {
        for (const auto& index_ratio : normalized_ratios) {
            for (const auto& curve : normalized_curves) {
                // A Fig. 11 adiciona uma dimensão extra em relação à Fig. 10:
                // cada família a/A5 é repetida para cada valor de n1/n5.
                Figure11CurveSummary curve_summary;
                curve_summary.curve = curve;
                curve_summary.index_ratio = index_ratio;
                curve_summary.solver_model = solver_model;
                curve_summary.total_points = config.point_count;

                for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                    const double c_over_a =
                        config.c_over_a_min + step * static_cast<double>(sample_index);

                    const auto point =
                        SolveFigure11Point(config, index_ratio, curve, solver_model, c_over_a);

                    if (sample_index == 0) {
                        // Mantém explícita a raiz transversal usada em toda a família.
                        curve_summary.kx_A5_over_pi = point.kx_A5_over_pi;
                    }

                    if (point.domain_valid) {
                        ++curve_summary.valid_points;
                    }

                    result.samples.push_back(
                        {
                            index_ratio.ratio_id,
                            index_ratio.label,
                            curve.curve_id,
                            curve.label,
                            sample_index,
                            c_over_a,
                            point
                        }
                    );
                }

                result.curves.push_back(curve_summary);
            }
        }
    }

    return result;
}

}  // namespace marcatili
