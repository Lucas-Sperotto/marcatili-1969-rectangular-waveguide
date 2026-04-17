#include "marcatili/physics/fig8.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "marcatili/math/waveguide_math.hpp"
#include "marcatili/physics/metal_guide.hpp"

namespace marcatili {
namespace {

using math::ComputeA;

void ValidateConfig(const Figure8Config& config) {
    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.a_over_b <= 0.0) {
        throw std::runtime_error("a_over_b must be positive.");
    }

    if (config.n1 <= config.n4) {
        throw std::runtime_error("The current Fig. 8 model requires n1 > n4.");
    }

    if (config.a_over_A_min <= 0.0 || config.a_over_A_max <= config.a_over_A_min) {
        throw std::runtime_error("a_over_A sweep bounds must define a positive interval.");
    }

    if (config.point_count < 2) {
        throw std::runtime_error("point_count must be at least 2.");
    }

    if (config.solver_models.empty()) {
        throw std::runtime_error("At least one solver model must be listed.");
    }

    if (config.modes.empty()) {
        throw std::runtime_error("At least one mode must be listed in the modes array.");
    }

}

std::string DefaultCurveId(const Figure8ModeSpec& mode) {
    return ToString(mode.family) + "_" + std::to_string(mode.p) + "_" + std::to_string(mode.q);
}

}  // namespace

Figure8ModeSpec ParseFigure8ModeSpec(const std::string& mode_text) {
    Figure8ModeSpec mode;

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
    mode.curve_id = DefaultCurveId(mode);
    return mode;
}

Figure8Result SolveFigure8(const Figure8Config& config) {
    ValidateConfig(config);

    Figure8Result result;
    result.config = config;
    result.status = "ok";

    const double A = ComputeA(config.wavelength, config.n1, config.n4);
    const double step =
        (config.a_over_A_max - config.a_over_A_min) /
        static_cast<double>(config.point_count - 1);

    for (const auto& solver_model : config.solver_models) {
        for (const auto& mode : config.modes) {
            Figure8CurveSummary curve_summary;
            curve_summary.mode = mode;
            curve_summary.solver_model = solver_model;
            curve_summary.total_points = config.point_count;

            for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                const double a_over_A =
                    config.a_over_A_min + step * static_cast<double>(sample_index);
                const double a = a_over_A * A;
                const double b = a / config.a_over_b;

                SingleGuideConfig point_config;
                point_config.case_id = config.case_id + "_" + mode.curve_id;
                point_config.article_target = config.article_target;
                point_config.csv_output_path = "";
                point_config.solver_model = solver_model;
                point_config.family = mode.family;
                point_config.p = mode.p;
                point_config.q = mode.q;
                point_config.wavelength = config.wavelength;
                point_config.a = a;
                point_config.b = b;
                point_config.n1 = config.n1;
                point_config.n2 = config.n4;
                point_config.n3 = config.n4;
                point_config.n4 = config.n4;
                point_config.n5 = config.n4;

                const auto point = SolveMetalGuide(point_config);

                if (point.domain_valid) {
                    ++curve_summary.valid_points;
                }
                if (point.guided) {
                    ++curve_summary.guided_points;
                }

                result.samples.push_back({mode.curve_id, sample_index, a_over_A, point});
            }

            result.curves.push_back(curve_summary);
        }
    }

    return result;
}

}  // namespace marcatili
