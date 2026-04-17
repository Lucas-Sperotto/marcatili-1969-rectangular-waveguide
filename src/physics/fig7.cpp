#include "marcatili/physics/fig7.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "marcatili/math/waveguide_math.hpp"

namespace marcatili {
namespace {

using math::ComputeA;

constexpr double kPi = 3.14159265358979323846;

double Square(double value) {
    return value * value;
}

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

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

void ValidateConfig(const Figure7Config& config) {
    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.a <= 0.0 || config.b <= 0.0) {
        throw std::runtime_error("a and b must be positive.");
    }

    if (config.line_point_count < 2) {
        throw std::runtime_error("line_point_count must be at least 2.");
    }

    if (config.modes.empty()) {
        throw std::runtime_error("At least one mode must be listed in the modes array.");
    }

    if (config.c_lines.empty()) {
        throw std::runtime_error("At least one C-line must be listed in the c_values array.");
    }

    const double external_max =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));
    if (config.n1 <= external_max) {
        throw std::runtime_error("Figure 7 requires n1 > n2, n3, n4, n5.");
    }

    for (const auto& c_line : config.c_lines) {
        if (c_line.c_value <= 0.0) {
            throw std::runtime_error("All C-line values must be positive.");
        }
    }
}

double ModeLineY(const Figure7ModeSpec& mode, double x) {
    return (1.0 - Square(static_cast<double>(mode.p)) * x) / Square(static_cast<double>(mode.q));
}

double CLineY(double c_value, double x) {
    return c_value * x;
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
        throw std::runtime_error(
            "Invalid mode specification: " + mode_text + ". Use family:p:q, e.g. E_y:1:1."
        );
    }

    mode.family = ParseSingleGuideFamily(family_text);
    mode.p = std::stoi(p_text);
    mode.q = std::stoi(q_text);
    mode.line_id =
        ToString(mode.family) + "_" + std::to_string(mode.p) + "_" + std::to_string(mode.q);
    return mode;
}

Figure7CLineSpec ParseFigure7CLineSpec(const std::string& c_text) {
    Figure7CLineSpec c_line;
    c_line.c_value = std::stod(c_text);
    c_line.line_id = "C=" + FormatCompactNumber(c_line.c_value);
    return c_line;
}

Figure7Result SolveFigure7(const Figure7Config& config) {
    ValidateConfig(config);

    Figure7Result result;
    result.config = config;
    result.status = "ok";

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

    const double x_denominator = 1.0 + (result.A3 + result.A5) / (kPi * config.a);
    const double y_denominator =
        1.0 +
        (Square(config.n2) * result.A2 + Square(config.n4) * result.A4) /
            (kPi * Square(config.n1) * config.b);

    // These numerators correspond directly to Eqs. (28) and (29):
    // X = x_numerator / (k1^2 - kz^2), Y = y_numerator / (k1^2 - kz^2).
    result.x_numerator = Square(kPi / config.a) / Square(x_denominator);
    result.y_numerator = Square(kPi / config.b) / Square(y_denominator);
    result.derived_c = Square((config.a / config.b) * (x_denominator / y_denominator));
    if (std::isfinite(config.reference_c_value)) {
        result.reference_c_absolute_error = std::abs(result.derived_c - config.reference_c_value);
        if (std::abs(config.reference_c_value) > 1e-15) {
            result.reference_c_relative_error =
                result.reference_c_absolute_error / std::abs(config.reference_c_value);
        }
    }
    result.design_example.a_over_b = config.a / config.b;

    const bool pair_35_symmetric = std::abs(config.n3 - config.n5) <= 1e-12;
    const bool pair_24_symmetric = std::abs(config.n2 - config.n4) <= 1e-12;
    result.design_example.symmetric_material_pairs = pair_35_symmetric && pair_24_symmetric;
    result.design_example.delta_from_n35 = 1.0 - 0.5 * (config.n3 + config.n5) / config.n1;
    result.design_example.delta_prime_from_n24 = 1.0 - 0.5 * (config.n2 + config.n4) / config.n1;

    if (result.design_example.delta_from_n35 > 0.0 &&
        result.design_example.delta_prime_from_n24 > 0.0) {
        result.design_example.sqrt_delta_prime_over_delta = std::sqrt(
            result.design_example.delta_prime_from_n24 / result.design_example.delta_from_n35
        );
    }

    for (const auto& mode : config.modes) {
        const double x_max = 1.0 / Square(static_cast<double>(mode.p));
        for (int sample_index = 0; sample_index < config.line_point_count; ++sample_index) {
            const double x = x_max * static_cast<double>(sample_index) /
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

    for (const auto& c_line : config.c_lines) {
        const double x_max = std::min(1.0, 1.0 / c_line.c_value);
        for (int sample_index = 0; sample_index < config.line_point_count; ++sample_index) {
            const double x = x_max * static_cast<double>(sample_index) /
                             static_cast<double>(config.line_point_count - 1);
            const double y = CLineY(c_line.c_value, x);
            if (y > 1.0) {
                continue;
            }

            result.line_samples.push_back(
                {"c_line", c_line.line_id, SingleGuideFamily::kEy, 0, 0, c_line.c_value, sample_index, x, y}
            );
        }

        for (const auto& mode : config.modes) {
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
                std::abs(c_line.c_value - config.reference_c_value) <= 1e-9;

            if (intersection.is_reference_c) {
                // Only the highlighted guide line is promoted to a physical kz evaluation.
                // For the other construction lines we keep the purely geometric intersection.
                const double kz_squared = Square(result.k1) - result.x_numerator / intersection.x;
                const double external_max_squared = std::max(
                    std::max(Square(result.k2), Square(result.k3)),
                    std::max(Square(result.k4), Square(result.k5))
                );

                intersection.domain_valid = kz_squared >= 0.0;
                if (intersection.domain_valid) {
                    intersection.kz = std::sqrt(kz_squared);
                    intersection.kz_normalized_against_n4 =
                        (kz_squared - Square(result.k4)) / (Square(result.k1) - Square(result.k4));
                    intersection.guided =
                        kz_squared > external_max_squared &&
                        intersection.kz_normalized_against_n4 >= 0.0 &&
                        intersection.kz_normalized_against_n4 <= 1.0;
                } else {
                    intersection.kz = NaN();
                    intersection.kz_normalized_against_n4 = NaN();
                    intersection.guided = false;
                }
            } else {
                intersection.domain_valid = false;
                intersection.guided = false;
                intersection.kz = NaN();
                intersection.kz_normalized_against_n4 = NaN();
            }

            result.intersections.push_back(intersection);
        }
    }

    if (!config.article_reference_mode_line_id.empty()) {
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

            if (std::isfinite(config.article_reference_y_readoff)) {
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

    return result;
}

}  // namespace marcatili
