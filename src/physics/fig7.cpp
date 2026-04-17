#include "marcatili/physics/fig7.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "marcatili/math/waveguide_math.hpp"

namespace marcatili {
namespace {

using math::ComputeA;
using math::kPi;
using math::Square;

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

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

std::string DefaultModeLineId(const Figure7ModeSpec& mode) {
    return ToString(mode.family) + "_" +
           std::to_string(mode.p) + "_" +
           std::to_string(mode.q);
}

std::string DefaultCLineId(const Figure7CLineSpec& c_line) {
    return "C=" + FormatCompactNumber(c_line.c_value);
}

void ValidateModeSpec(const Figure7ModeSpec& mode) {
    if (mode.p <= 0 || mode.q <= 0) {
        throw std::invalid_argument(
            "SolveFigure7: mode indices p and q must be positive integers."
        );
    }
}

void ValidateCLineSpec(const Figure7CLineSpec& c_line) {
    if (!IsFiniteNumber(c_line.c_value) || c_line.c_value <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure7: all C-line values must be finite positive values."
        );
    }
}

Figure7ModeSpec NormalizeModeSpec(const Figure7ModeSpec& mode) {
    ValidateModeSpec(mode);

    Figure7ModeSpec normalized = mode;
    if (normalized.line_id.empty()) {
        normalized.line_id = DefaultModeLineId(normalized);
    }

    return normalized;
}

Figure7CLineSpec NormalizeCLineSpec(const Figure7CLineSpec& c_line) {
    ValidateCLineSpec(c_line);

    Figure7CLineSpec normalized = c_line;
    if (normalized.line_id.empty()) {
        normalized.line_id = DefaultCLineId(normalized);
    }

    return normalized;
}

void ValidateConfig(const Figure7Config& config) {
    if (!IsFiniteNumber(config.wavelength) || config.wavelength <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure7: wavelength must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.a) || !IsFiniteNumber(config.b) ||
        config.a <= 0.0 || config.b <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure7: a and b must be finite positive values."
        );
    }

    if (!IsFiniteNumber(config.n1) || config.n1 <= 0.0 ||
        !IsFiniteNumber(config.n2) || config.n2 <= 0.0 ||
        !IsFiniteNumber(config.n3) || config.n3 <= 0.0 ||
        !IsFiniteNumber(config.n4) || config.n4 <= 0.0 ||
        !IsFiniteNumber(config.n5) || config.n5 <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure7: refractive indices n1..n5 must be finite positive values."
        );
    }

    if (config.line_point_count < 2) {
        throw std::invalid_argument(
            "SolveFigure7: line_point_count must be at least 2."
        );
    }

    if (config.modes.empty()) {
        throw std::invalid_argument(
            "SolveFigure7: at least one mode must be listed."
        );
    }

    if (config.c_lines.empty()) {
        throw std::invalid_argument(
            "SolveFigure7: at least one C-line must be listed."
        );
    }

    const double external_max =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));

    if (!(config.n1 > external_max)) {
        throw std::invalid_argument(
            "SolveFigure7: requires n1 > n2, n3, n4 and n5."
        );
    }

    for (const auto& mode : config.modes) {
        ValidateModeSpec(mode);
    }

    for (const auto& c_line : config.c_lines) {
        ValidateCLineSpec(c_line);
    }
}

double ModeLineY(const Figure7ModeSpec& mode, double x) {
    // No nomograma, as famílias modais aparecem como retas no plano (X, Y).
    return (1.0 - Square(static_cast<double>(mode.p)) * x) /
           Square(static_cast<double>(mode.q));
}

double CLineY(double c_value, double x) {
    // Reta de construção Y = C X.
    return c_value * x;
}

void AppendModeLineSamples(
    const Figure7Config& config,
    const Figure7ModeSpec& mode,
    Figure7Result& result
) {
    const double x_max = 1.0 / Square(static_cast<double>(mode.p));

    for (int sample_index = 0; sample_index < config.line_point_count; ++sample_index) {
        const double x =
            x_max * static_cast<double>(sample_index) /
            static_cast<double>(config.line_point_count - 1);

        const double y = ModeLineY(mode, x);
        if (y < 0.0) {
            continue;
        }

        result.line_samples.push_back(
            {"mode", mode.line_id, mode.family, mode.p, mode.q, NaN(), sample_index, x, y}
        );
    }
}

void AppendCLineSamples(
    const Figure7Config& config,
    const Figure7CLineSpec& c_line,
    Figure7Result& result
) {
    const double x_max = std::min(1.0, 1.0 / c_line.c_value);

    for (int sample_index = 0; sample_index < config.line_point_count; ++sample_index) {
        const double x =
            x_max * static_cast<double>(sample_index) /
            static_cast<double>(config.line_point_count - 1);

        const double y = CLineY(c_line.c_value, x);
        if (y > 1.0) {
            continue;
        }

        result.line_samples.push_back(
            {"c_line", c_line.line_id, SingleGuideFamily::kEy, 0, 0, c_line.c_value, sample_index, x, y}
        );
    }
}

void EvaluateReferenceIntersection(
    const Figure7Result& base_result,
    Figure7Intersection& intersection
) {
    const double kz_squared =
        Square(base_result.k1) - base_result.x_numerator / intersection.x;

    const double external_max_squared = std::max(
        std::max(Square(base_result.k2), Square(base_result.k3)),
        std::max(Square(base_result.k4), Square(base_result.k5))
    );

    intersection.domain_valid = IsFiniteNumber(kz_squared) && kz_squared >= 0.0;
    if (!intersection.domain_valid) {
        intersection.kz = NaN();
        intersection.kz_normalized_against_n4 = NaN();
        intersection.guided = false;
        return;
    }

    intersection.kz = std::sqrt(kz_squared);

    const double normalization_denominator =
        Square(base_result.k1) - Square(base_result.k4);

    intersection.kz_normalized_against_n4 =
        (normalization_denominator > 0.0)
            ? (kz_squared - Square(base_result.k4)) / normalization_denominator
            : NaN();

    intersection.guided =
        kz_squared > external_max_squared &&
        IsFiniteNumber(intersection.kz_normalized_against_n4) &&
        intersection.kz_normalized_against_n4 >= 0.0 &&
        intersection.kz_normalized_against_n4 <= 1.0;
}

void AppendIntersectionsForCLine(
    const Figure7Config& config,
    const std::vector<Figure7ModeSpec>& modes,
    const Figure7CLineSpec& c_line,
    Figure7Result& result
) {
    for (const auto& mode : modes) {
        Figure7Intersection intersection;
        intersection.mode_line_id = mode.line_id;
        intersection.c_line_id = c_line.line_id;
        intersection.family = mode.family;
        intersection.p = mode.p;
        intersection.q = mode.q;
        intersection.c_value = c_line.c_value;

        intersection.x =
            1.0 / (Square(static_cast<double>(mode.p)) +
                   Square(static_cast<double>(mode.q)) * c_line.c_value);
        intersection.y = c_line.c_value * intersection.x;

        intersection.is_reference_c =
            IsFiniteNumber(config.reference_c_value) &&
            std::abs(c_line.c_value - config.reference_c_value) <= 1e-9;

        if (intersection.is_reference_c) {
            // Apenas a reta de referência é promovida de construção geométrica
            // para avaliação física via kz.
            EvaluateReferenceIntersection(result, intersection);
        } else {
            intersection.domain_valid = false;
            intersection.guided = false;
            intersection.kz = NaN();
            intersection.kz_normalized_against_n4 = NaN();
        }

        result.intersections.push_back(intersection);
    }
}

void BuildArticleReferenceCheck(
    const Figure7Config& config,
    Figure7Result& result
) {
    if (config.article_reference_mode_line_id.empty()) {
        return;
    }

    for (const auto& intersection : result.intersections) {
        if (!intersection.is_reference_c ||
            intersection.mode_line_id != config.article_reference_mode_line_id) {
            continue;
        }

        result.article_reference_check.available = true;
        result.article_reference_check.mode_line_id = intersection.mode_line_id;
        result.article_reference_check.note = config.article_reference_note;
        result.article_reference_check.c_value = intersection.c_value;
        result.article_reference_check.exact_x = intersection.x;
        result.article_reference_check.exact_y = intersection.y;
        result.article_reference_check.article_y_readoff =
            config.article_reference_y_readoff;

        if (IsFiniteNumber(config.article_reference_y_readoff)) {
            result.article_reference_check.article_y_absolute_error =
                std::abs(intersection.y - config.article_reference_y_readoff);

            if (std::abs(config.article_reference_y_readoff) > 1e-15) {
                result.article_reference_check.article_y_relative_error =
                    result.article_reference_check.article_y_absolute_error /
                    std::abs(config.article_reference_y_readoff);
            }
        }

        break;
    }
}

}  // namespace

Figure7ModeSpec ParseFigure7ModeSpec(const std::string& mode_text) {
    Figure7ModeSpec mode;

    std::stringstream parser(mode_text);
    std::string family_text;
    std::string p_text;
    std::string q_text;

    if (!std::getline(parser, family_text, ':') ||
        !std::getline(parser, p_text, ':') ||
        !std::getline(parser, q_text, ':')) {
        throw std::invalid_argument(
            "ParseFigure7ModeSpec: invalid mode specification '" + mode_text +
            "'. Use family:p:q, for example E_y:1:1."
        );
    }

    mode.family = ParseSingleGuideFamily(family_text);

    try {
        mode.p = std::stoi(p_text);
        mode.q = std::stoi(q_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure7ModeSpec: invalid numeric indices in '" + mode_text + "'."
        );
    }

    mode = NormalizeModeSpec(mode);
    return mode;
}

Figure7CLineSpec ParseFigure7CLineSpec(const std::string& c_text) {
    Figure7CLineSpec c_line;

    try {
        c_line.c_value = std::stod(c_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure7CLineSpec: invalid C-line specification '" + c_text +
            "'. Expected a numeric value."
        );
    }

    c_line = NormalizeCLineSpec(c_line);
    return c_line;
}

Figure7Result SolveFigure7(const Figure7Config& config) {
    ValidateConfig(config);

    Figure7Result result;
    result.config = config;
    result.status = "ok";

    std::vector<Figure7ModeSpec> normalized_modes;
    normalized_modes.reserve(config.modes.size());
    for (const auto& mode : config.modes) {
        normalized_modes.push_back(NormalizeModeSpec(mode));
    }

    std::vector<Figure7CLineSpec> normalized_c_lines;
    normalized_c_lines.reserve(config.c_lines.size());
    for (const auto& c_line : config.c_lines) {
        normalized_c_lines.push_back(NormalizeCLineSpec(c_line));
    }

    result.k0 = 2.0 * kPi / config.wavelength;
    result.k1 = result.k0 * config.n1;
    result.k2 = result.k0 * config.n2;
    result.k3 = result.k0 * config.n3;
    result.k4 = result.k0 * config.n4;
    result.k5 = result.k0 * config.n5;

    result.A2 = ComputeA(config.wavelength, config.n1, config.n2);
    result.A3 = ComputeA(config.wavelength, config.n1, config.n3);
    result.A4 = ComputeA(config.wavelength, config.n1, config.n4);
    result.A5 = ComputeA(config.wavelength, config.n1, config.n5);

    // A Fig. 7 é construída a partir do modelo fechado de Marcatili.
    const double x_denominator =
        1.0 + (result.A3 + result.A5) / (kPi * config.a);

    const double y_denominator =
        1.0 +
        (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
            (kPi * Square(config.n1) * config.b);

    // Eqs. equivalentes a (28) e (29) no nomograma.
    result.x_numerator = Square(kPi / config.a) / Square(x_denominator);
    result.y_numerator = Square(kPi / config.b) / Square(y_denominator);

    result.derived_c =
        Square((config.a / config.b) * (x_denominator / y_denominator));

    if (IsFiniteNumber(config.reference_c_value)) {
        result.reference_c_absolute_error =
            std::abs(result.derived_c - config.reference_c_value);

        if (std::abs(config.reference_c_value) > 1e-15) {
            result.reference_c_relative_error =
                result.reference_c_absolute_error / std::abs(config.reference_c_value);
        }
    }

    result.design_example.a_over_b = config.a / config.b;

    const bool pair_35_symmetric = std::abs(config.n3 - config.n5) <= 1e-12;
    const bool pair_24_symmetric = std::abs(config.n2 - config.n4) <= 1e-12;
    result.design_example.symmetric_material_pairs =
        pair_35_symmetric && pair_24_symmetric;

    result.design_example.delta_from_n35 =
        1.0 - 0.5 * (config.n3 + config.n5) / config.n1;
    result.design_example.delta_prime_from_n24 =
        1.0 - 0.5 * (config.n2 + config.n4) / config.n1;

    if (result.design_example.delta_from_n35 > 0.0 &&
        result.design_example.delta_prime_from_n24 > 0.0) {
        result.design_example.sqrt_delta_prime_over_delta =
            std::sqrt(
                result.design_example.delta_prime_from_n24 /
                result.design_example.delta_from_n35
            );
    }

    for (const auto& mode : normalized_modes) {
        AppendModeLineSamples(config, mode, result);
    }

    for (const auto& c_line : normalized_c_lines) {
        AppendCLineSamples(config, c_line, result);
        AppendIntersectionsForCLine(config, normalized_modes, c_line, result);
    }

    BuildArticleReferenceCheck(config, result);

    return result;
}

}  // namespace marcatili
