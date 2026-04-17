#include "marcatili/physics/fig6.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "marcatili/math/waveguide_math.hpp"
#include "marcatili/physics/slab_guide.hpp"

namespace marcatili {
namespace {

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool IsFiniteNumber(double value) {
    return std::isfinite(value);
}

void ValidateMaterialVariant(const Figure6MaterialVariant& variant, double n1) {
    if (variant.variant_id.empty()) {
        throw std::invalid_argument(
            "SolveFigure6: material_variants entries must provide a non-empty variant_id."
        );
    }

    if (!IsFiniteNumber(variant.n2) || variant.n2 <= 0.0 ||
        !IsFiniteNumber(variant.n3) || variant.n3 <= 0.0 ||
        !IsFiniteNumber(variant.n4) || variant.n4 <= 0.0 ||
        !IsFiniteNumber(variant.n5) || variant.n5 <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure6: material variant refractive indices must be finite positive values."
        );
    }

    if (!(n1 > variant.n4)) {
        throw std::invalid_argument(
            "SolveFigure6: each material variant must satisfy n1 > n4 so that A4 is real."
        );
    }
}

void ValidateModeSpec(const Figure6ModeSpec& mode) {
    if (mode.p <= 0 || mode.q <= 0) {
        throw std::invalid_argument(
            "SolveFigure6: mode indices p and q must be positive integers."
        );
    }
}

void ValidateConfig(const Figure6Config& config) {
    if (!IsFiniteNumber(config.wavelength) || config.wavelength <= 0.0) {
        throw std::invalid_argument("SolveFigure6: wavelength must be a finite positive value.");
    }

    if (!IsFiniteNumber(config.n1) || config.n1 <= 0.0) {
        throw std::invalid_argument("SolveFigure6: n1 must be a finite positive value.");
    }

    if (config.geometry_model == Figure6GeometryModel::kRectangular) {
        if (!IsFiniteNumber(config.a_over_b) || config.a_over_b <= 0.0) {
            throw std::invalid_argument(
                "SolveFigure6: a_over_b must be a finite positive value for rectangular geometry."
            );
        }
    }

    if (!IsFiniteNumber(config.b_over_A4_min) || !IsFiniteNumber(config.b_over_A4_max) ||
        config.b_over_A4_min <= 0.0 || config.b_over_A4_max <= 0.0) {
        throw std::invalid_argument(
            "SolveFigure6: b_over_A4 sweep bounds must be finite positive values."
        );
    }

    if (config.b_over_A4_max <= config.b_over_A4_min) {
        throw std::invalid_argument(
            "SolveFigure6: b_over_A4_max must be greater than b_over_A4_min."
        );
    }

    if (config.point_count < 2) {
        throw std::invalid_argument("SolveFigure6: point_count must be at least 2.");
    }

    if (config.modes.empty()) {
        throw std::invalid_argument("SolveFigure6: at least one mode must be listed.");
    }

    if (config.solver_models.empty()) {
        throw std::invalid_argument("SolveFigure6: at least one solver model must be listed.");
    }

    if (config.material_variants.empty()) {
        if (!IsFiniteNumber(config.n2) || config.n2 <= 0.0 ||
            !IsFiniteNumber(config.n3) || config.n3 <= 0.0 ||
            !IsFiniteNumber(config.n4) || config.n4 <= 0.0 ||
            !IsFiniteNumber(config.n5) || config.n5 <= 0.0) {
            throw std::invalid_argument(
                "SolveFigure6: default refractive indices n2, n3, n4 and n5 must be finite positive values."
            );
        }

        if (!(config.n1 > config.n4)) {
            throw std::invalid_argument(
                "SolveFigure6: requires n1 > n4 so that A4 is real."
            );
        }
    }

    for (const auto& mode : config.modes) {
        ValidateModeSpec(mode);
    }

    for (const auto& variant : config.material_variants) {
        ValidateMaterialVariant(variant, config.n1);
    }
}

std::string DefaultCurveId(const Figure6ModeSpec& mode) {
    std::ostringstream curve;
    curve << ToString(mode.family) << "_" << mode.p << "_" << mode.q;
    return curve.str();
}

Figure6ModeSpec NormalizeModeSpec(const Figure6ModeSpec& mode) {
    ValidateModeSpec(mode);

    Figure6ModeSpec normalized = mode;
    if (normalized.curve_id.empty()) {
        normalized.curve_id = DefaultCurveId(normalized);
    }

    return normalized;
}

Figure6MaterialVariant BuildDefaultVariant(const Figure6Config& config) {
    Figure6MaterialVariant variant;
    variant.variant_id = "default";
    variant.n2 = config.n2;
    variant.n3 = config.n3;
    variant.n4 = config.n4;
    variant.n5 = config.n5;
    return variant;
}

SingleGuideResult SolveFigure6Point(
    const Figure6Config& config,
    const Figure6MaterialVariant& variant,
    const Figure6ModeSpec& mode,
    SingleGuideSolverModel solver_model,
    double b
) {
    SingleGuideConfig point_config;
    point_config.case_id =
        config.case_id + "_" + variant.variant_id + "_" + mode.curve_id + "_" + ToString(solver_model);
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = solver_model;
    point_config.family = mode.family;
    point_config.p = mode.p;
    point_config.q = mode.q;
    point_config.wavelength = config.wavelength;
    point_config.a =
        (config.geometry_model == Figure6GeometryModel::kRectangular)
            ? config.a_over_b * b
            : NaN();
    point_config.b = b;
    point_config.n1 = config.n1;
    point_config.n2 = variant.n2;
    point_config.n3 = variant.n3;
    point_config.n4 = variant.n4;
    point_config.n5 = variant.n5;

    // A Fig. 6 é construída como um sweep repetido de solves pontuais.
    // A física modal permanece encapsulada em SolveSingleGuide / SolveSlabGuide.
    if (config.geometry_model == Figure6GeometryModel::kSlab) {
        return SolveSlabGuide(point_config);
    }

    return SolveSingleGuide(point_config);
}

}  // namespace

std::string ToString(Figure6GeometryModel geometry_model) {
    switch (geometry_model) {
        case Figure6GeometryModel::kRectangular:
            return "rectangular";
        case Figure6GeometryModel::kSlab:
            return "slab";
    }

    return "unknown";
}

Figure6GeometryModel ParseFigure6GeometryModel(const std::string& geometry_model_text) {
    if (geometry_model_text.empty() || geometry_model_text == "rectangular") {
        return Figure6GeometryModel::kRectangular;
    }

    if (geometry_model_text == "slab" || geometry_model_text == "slab_limit") {
        return Figure6GeometryModel::kSlab;
    }

    throw std::invalid_argument(
        "ParseFigure6GeometryModel: supported values are rectangular and slab."
    );
}

Figure6ModeSpec ParseFigure6ModeSpec(const std::string& mode_text) {
    Figure6ModeSpec mode;

    std::stringstream parser(mode_text);
    std::string family_text;
    std::string p_text;
    std::string q_text;

    if (!std::getline(parser, family_text, ':') ||
        !std::getline(parser, p_text, ':') ||
        !std::getline(parser, q_text, ':')) {
        throw std::invalid_argument(
            "ParseFigure6ModeSpec: invalid mode specification '" + mode_text +
            "'. Use family:p:q, for example E_y:1:1."
        );
    }

    mode.family = ParseSingleGuideFamily(family_text);

    try {
        mode.p = std::stoi(p_text);
        mode.q = std::stoi(q_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseFigure6ModeSpec: invalid numeric indices in '" + mode_text + "'."
        );
    }

    mode = NormalizeModeSpec(mode);
    return mode;
}

Figure6Result SolveFigure6(const Figure6Config& config) {
    ValidateConfig(config);

    Figure6Result result;
    result.config = config;
    result.status = "ok";

    std::vector<Figure6MaterialVariant> variants = config.material_variants;
    if (variants.empty()) {
        variants.push_back(BuildDefaultVariant(config));
    }

    std::vector<Figure6ModeSpec> normalized_modes;
    normalized_modes.reserve(config.modes.size());
    for (const auto& mode : config.modes) {
        normalized_modes.push_back(NormalizeModeSpec(mode));
    }

    for (const auto& variant : variants) {
        // O eixo horizontal da figura é b / A4. Recomputar A4 por variante
        // preserva corretamente a normalização quando n4 muda entre subcasos.
        const double A4 = math::ComputeA(config.wavelength, config.n1, variant.n4);
        const double step =
            (config.b_over_A4_max - config.b_over_A4_min) /
            static_cast<double>(config.point_count - 1);

        for (const auto& solver_model : config.solver_models) {
            for (const auto& mode : normalized_modes) {
                Figure6CurveSummary curve_summary;
                curve_summary.variant_id = variant.variant_id;
                curve_summary.mode = mode;
                curve_summary.solver_model = solver_model;
                curve_summary.total_points = config.point_count;

                for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                    const double b_over_A4 =
                        config.b_over_A4_min + step * static_cast<double>(sample_index);
                    const double b = b_over_A4 * A4;

                    const auto point =
                        SolveFigure6Point(config, variant, mode, solver_model, b);

                    if (point.domain_valid) {
                        ++curve_summary.valid_points;
                    }

                    if (point.guided) {
                        ++curve_summary.guided_points;
                    }

                    result.samples.push_back(
                        {config.panel_id, variant.variant_id, mode.curve_id, sample_index, b_over_A4, point}
                    );
                }

                result.curves.push_back(curve_summary);
            }
        }
    }

    return result;
}

}  // namespace marcatili
