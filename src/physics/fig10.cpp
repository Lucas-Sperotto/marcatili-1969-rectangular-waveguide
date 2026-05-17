#include "marcatili/physics/fig10.hpp"

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

Figure10CurveSpec NormalizeCurveSpec(const Figure10CurveSpec& curve) {
    if (!IsFiniteNumber(curve.a_over_A5) || curve.a_over_A5 <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure10: each curve must provide a finite positive a_over_A5."
        );
    }

    Figure10CurveSpec normalized = curve;

    if (normalized.label.empty()) {
        normalized.label = FormatCompactNumber(normalized.a_over_A5);
    }

    if (normalized.curve_id.empty()) {
        normalized.curve_id = "a_over_A5=" + normalized.label;
    }

    return normalized;
}

Figure10Config NormalizeConfig(const Figure10Config& config) {
    Figure10Config normalized = config;

    if (normalized.transverse_equation == CouplerTransverseEquation::kEq20) {
        const bool has_n1_over_n5 =
            IsFiniteNumber(normalized.n1_over_n5) && normalized.n1_over_n5 > 1.0;

        const bool has_index_ratio_squared =
            IsFiniteNumber(normalized.index_ratio_squared) &&
            normalized.index_ratio_squared > 0.0 &&
            normalized.index_ratio_squared < 1.0;

        if (!has_n1_over_n5 && !has_index_ratio_squared) {
            throw std::invalid_argument(
                "SolveFigure10: Eq. (20) requires n1_over_n5 > 1 or "
                "0 < index_ratio_squared < 1."
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
                    "SolveFigure10: inconsistent n1_over_n5 and index_ratio_squared."
                );
            }
        }
    }

    return normalized;
}

void ValidateConfig(const Figure10Config& config) {
    if (!IsFiniteNumber(config.c_over_a_min) || !IsFiniteNumber(config.c_over_a_max)) {
        throw std::invalid_argument(
            "SolveFigure10: c_over_a sweep bounds must be finite."
        );
    }

    if (config.c_over_a_min < 0.0 || config.c_over_a_max <= config.c_over_a_min) {
        throw std::invalid_argument(
            "SolveFigure10: c_over_a sweep bounds must define a non-negative interval."
        );
    }

    if (config.point_count < 2) {
        throw std::invalid_argument(
            "SolveFigure10: point_count must be at least 2."
        );
    }

    if (config.solver_models.empty()) {
        throw std::invalid_argument(
            "SolveFigure10: at least one solver model must be listed."
        );
    }

    if (config.curves.empty()) {
        throw std::invalid_argument(
            "SolveFigure10: at least one curve must be listed."
        );
    }

    if (config.transverse_equation == CouplerTransverseEquation::kEq20 &&
        !(config.index_ratio_squared > 0.0 && config.index_ratio_squared < 1.0)) {
        throw std::invalid_argument(
            "SolveFigure10: Eq. (20) requires 0 < index_ratio_squared < 1."
        );
    }

    for (const auto& curve : config.curves) {
        NormalizeCurveSpec(curve);
    }
}

CouplerPointResult SolveFigure10Point(
    const Figure10Config& config,
    const Figure10CurveSpec& curve,
    SingleGuideSolverModel solver_model,
    double c_over_a
) {
    CouplerPointConfig point_config;
    point_config.case_id =
        config.case_id + "_" + curve.curve_id + "_" + ToString(solver_model);
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = solver_model;

    // O caso-base usa Eq. (6)/(12); testes alternativos podem selecionar Eq. (20).
    point_config.transverse_equation = config.transverse_equation;
    point_config.p = 1;
    point_config.a_over_A5 = curve.a_over_A5;
    point_config.c_over_a = c_over_a;
    point_config.index_ratio_squared = config.index_ratio_squared;

    return SolveCouplerPoint(point_config);
}

}  // namespace

Figure10CurveSpec ParseFigure10CurveSpec(const std::string& value_text) {
    Figure10CurveSpec curve;

    try {
        curve.a_over_A5 = std::stod(value_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure10CurveSpec: invalid curve specification '" + value_text +
            "'. Expected a numeric value for a_over_A5."
        );
    }

    curve = NormalizeCurveSpec(curve);
    return curve;
}

Figure10Result SolveFigure10(const Figure10Config& config) {
    const Figure10Config normalized_config = NormalizeConfig(config);
    ValidateConfig(normalized_config);

    Figure10Result result;
    result.config = normalized_config;
    result.status = "ok";

    std::vector<Figure10CurveSpec> normalized_curves;
    normalized_curves.reserve(normalized_config.curves.size());
    for (const auto& curve : normalized_config.curves) {
        normalized_curves.push_back(NormalizeCurveSpec(curve));
    }

    const double step =
        (normalized_config.c_over_a_max - normalized_config.c_over_a_min) /
        static_cast<double>(normalized_config.point_count - 1);

    for (const auto& solver_model : normalized_config.solver_models) {
        for (const auto& curve : normalized_curves) {
            Figure10CurveSummary curve_summary;
            curve_summary.curve = curve;
            curve_summary.solver_model = solver_model;
            curve_summary.total_points = normalized_config.point_count;

            for (
                int sample_index = 0;
                sample_index < normalized_config.point_count;
                ++sample_index
            ) {
                const double c_over_a =
                    normalized_config.c_over_a_min + step * static_cast<double>(sample_index);

                // Cada família da Fig. 10 fixa a / A5 e varre c / a.
                // A expressão de acoplamento fica centralizada em SolveCouplerPoint.
                const auto point =
                    SolveFigure10Point(normalized_config, curve, solver_model, c_over_a);

                if (sample_index == 0) {
                    // Guardamos a raiz transversal usada em toda a família
                    // para tornar o relatório mais pedagógico.
                    curve_summary.kx_A5_over_pi = point.kx_A5_over_pi;
                }

                if (point.domain_valid) {
                    ++curve_summary.valid_points;
                }

                result.samples.push_back(
                    {curve.curve_id, curve.label, sample_index, c_over_a, point}
                );
            }

            result.curves.push_back(curve_summary);
        }
    }

    return result;
}

}  // namespace marcatili
