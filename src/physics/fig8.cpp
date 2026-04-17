#include "marcatili/physics/fig8.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "marcatili/math/waveguide_math.hpp"
#include "marcatili/physics/metal_guide.hpp"

namespace marcatili {
namespace {

using math::ComputeA;

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

void ValidateModeSpec(const Figure8ModeSpec& mode) {
    if (mode.p <= 0 || mode.q <= 0) {
        throw std::invalid_argument(
            "SolveFigure8: mode indices p and q must be positive integers."
        );
    }
}

std::string DefaultCurveId(const Figure8ModeSpec& mode) {
    return ToString(mode.family) + "_" +
           std::to_string(mode.p) + "_" +
           std::to_string(mode.q);
}

Figure8ModeSpec NormalizeModeSpec(const Figure8ModeSpec& mode) {
    ValidateModeSpec(mode);

    Figure8ModeSpec normalized = mode;
    if (normalized.curve_id.empty()) {
        normalized.curve_id = DefaultCurveId(normalized);
    }

    return normalized;
}

void ValidateConfig(const Figure8Config& config) {
    if (!IsFiniteNumber(config.wavelength) || config.wavelength <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure8: wavelength must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.a_over_b) || config.a_over_b <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure8: a_over_b must be a finite positive value."
        );
    }

    if (!IsFiniteNumber(config.n1) || config.n1 <= 0.0 ||
        !IsFiniteNumber(config.n4) || config.n4 <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure8: n1 and n4 must be finite positive values."
        );
    }

    if (!(config.n1 > config.n4)) {
        throw std::invalid_argument(
            "SolveFigure8: the current Fig. 8 model requires n1 > n4."
        );
    }

    if (!IsFiniteNumber(config.a_over_A_min) || !IsFiniteNumber(config.a_over_A_max)) {
        throw std::invalid_argument(
            "SolveFigure8: a_over_A sweep bounds must be finite."
        );
    }

    if (config.a_over_A_min <= 0.0 || config.a_over_A_max <= config.a_over_A_min) {
        throw std::invalid_argument(
            "SolveFigure8: a_over_A sweep bounds must define a positive interval."
        );
    }

    if (config.point_count < 2) {
        throw std::invalid_argument(
            "SolveFigure8: point_count must be at least 2."
        );
    }

    if (config.solver_models.empty()) {
        throw std::invalid_argument(
            "SolveFigure8: at least one solver model must be listed."
        );
    }

    if (config.modes.empty()) {
        throw std::invalid_argument(
            "SolveFigure8: at least one mode must be listed."
        );
    }

    for (const auto& mode : config.modes) {
        ValidateModeSpec(mode);
    }
}

SingleGuideResult SolveFigure8Point(
    const Figure8Config& config,
    const Figure8ModeSpec& mode,
    SingleGuideSolverModel solver_model,
    double a,
    double b
) {
    SingleGuideConfig point_config;
    point_config.case_id =
        config.case_id + "_" + mode.curve_id + "_" + ToString(solver_model);
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = solver_model;
    point_config.family = mode.family;
    point_config.p = mode.p;
    point_config.q = mode.q;
    point_config.wavelength = config.wavelength;
    point_config.a = a;
    point_config.b = b;

    // Na implementação atual da Fig. 8, o meio externo é tratado com o mesmo
    // índice em todas as regiões exteriores e a interface relevante é a
    // metalizada, resolvida por SolveMetalGuide.
    point_config.n1 = config.n1;
    point_config.n2 = config.n4;
    point_config.n3 = config.n4;
    point_config.n4 = config.n4;
    point_config.n5 = config.n4;

    return SolveMetalGuide(point_config);
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
        throw std::invalid_argument(
            "ParseFigure8ModeSpec: invalid mode specification '" + mode_text +
            "'. Use family:p:q, for example E_y:1:1."
        );
    }

    mode.family = ParseSingleGuideFamily(family_text);

    try {
        mode.p = std::stoi(p_text);
        mode.q = std::stoi(q_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure8ModeSpec: invalid numeric indices in '" + mode_text + "'."
        );
    }

    mode = NormalizeModeSpec(mode);
    return mode;
}

Figure8Result SolveFigure8(const Figure8Config& config) {
    ValidateConfig(config);

    Figure8Result result;
    result.config = config;
    result.status = "ok";

    std::vector<Figure8ModeSpec> normalized_modes;
    normalized_modes.reserve(config.modes.size());
    for (const auto& mode : config.modes) {
        normalized_modes.push_back(NormalizeModeSpec(mode));
    }

    // A variável horizontal da figura é a / A, com A calculado a partir de n4.
    // Isso mantém o sweep na mesma coordenada adimensional usada no artigo.
    const double A = ComputeA(config.wavelength, config.n1, config.n4);
    const double step =
        (config.a_over_A_max - config.a_over_A_min) /
        static_cast<double>(config.point_count - 1);

    for (const auto& solver_model : config.solver_models) {
        for (const auto& mode : normalized_modes) {
            Figure8CurveSummary curve_summary;
            curve_summary.mode = mode;
            curve_summary.solver_model = solver_model;
            curve_summary.total_points = config.point_count;

            for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                const double a_over_A =
                    config.a_over_A_min + step * static_cast<double>(sample_index);
                const double a = a_over_A * A;
                const double b = a / config.a_over_b;

                // O driver da figura apenas monta o sweep e delega a física
                // ao solver do guia metalizado.
                const auto point =
                    SolveFigure8Point(config, mode, solver_model, a, b);

                if (point.domain_valid) {
                    ++curve_summary.valid_points;
                }

                if (point.guided) {
                    ++curve_summary.guided_points;
                }

                result.samples.push_back(
                    {mode.curve_id, sample_index, a_over_A, point}
                );
            }

            result.curves.push_back(curve_summary);
        }
    }

    return result;
}

}  // namespace marcatili
