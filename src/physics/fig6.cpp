#include "marcatili/physics/fig6.hpp"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "marcatili/physics/slab_guide.hpp"

namespace marcatili {
namespace {

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

void ValidateConfig(const Figure6Config& config) {
    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.geometry_model == Figure6GeometryModel::kRectangular && config.a_over_b <= 0.0) {
        throw std::runtime_error("a_over_b must be positive.");
    }

    if (config.b_over_A4_min <= 0.0 || config.b_over_A4_max <= 0.0) {
        throw std::runtime_error("b_over_A4 sweep bounds must be positive.");
    }

    if (config.b_over_A4_max <= config.b_over_A4_min) {
        throw std::runtime_error("b_over_A4_max must be greater than b_over_A4_min.");
    }

    if (config.point_count < 2) {
        throw std::runtime_error("point_count must be at least 2.");
    }

    if (config.modes.empty()) {
        throw std::runtime_error("At least one mode must be listed in the modes array.");
    }

    if (config.solver_models.empty()) {
        throw std::runtime_error("At least one solver model must be listed.");
    }

    for (const auto& variant : config.material_variants) {
        if (variant.variant_id.empty()) {
            throw std::runtime_error("material_variants entries must provide a non-empty variant id.");
        }
    }
}

std::string DefaultCurveId(const Figure6ModeSpec& mode) {
    std::ostringstream curve;
    curve << ToString(mode.family) << "_" << mode.p << "_" << mode.q;
    return curve.str();
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
        config.case_id + "_" + mode.curve_id + "_" + ToString(solver_model);
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = solver_model;
    point_config.family = mode.family;
    point_config.p = mode.p;
    point_config.q = mode.q;
    point_config.wavelength = config.wavelength;
    point_config.a =
        config.geometry_model == Figure6GeometryModel::kRectangular ? config.a_over_b * b : NaN();
    point_config.b = b;
    point_config.n1 = config.n1;
    point_config.n2 = variant.n2;
    point_config.n3 = variant.n3;
    point_config.n4 = variant.n4;
    point_config.n5 = variant.n5;

    // Fig. 6 is generated as a repeated single-point solve over a normalized sweep.
    // The figure builder itself does not contain modal physics; it only assembles
    // the correct per-point configuration for the selected panel.
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

    throw std::runtime_error(
        "Unsupported geometry_model. Use one of: rectangular, slab."
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

Figure6Result SolveFigure6(const Figure6Config& config) {
    ValidateConfig(config);

    Figure6Result result;
    result.config = config;
    result.status = "ok";

    std::vector<Figure6MaterialVariant> variants = config.material_variants;
    if (variants.empty()) {
        variants.push_back({"default", config.n2, config.n3, config.n4, config.n5});
    }

    for (const auto& variant : variants) {
        // Each panel is swept in the article's normalized horizontal coordinate b/A_4.
        // Recomputing A_4 per material variant keeps that normalization faithful when
        // n4 changes across subcases such as Fig. 6d.
        const double a4 =
            config.wavelength /
            (2.0 * std::sqrt(config.n1 * config.n1 - variant.n4 * variant.n4));
        const double step =
            (config.b_over_A4_max - config.b_over_A4_min) /
            static_cast<double>(config.point_count - 1);

        for (const auto& solver_model : config.solver_models) {
            for (const auto& mode : config.modes) {
                Figure6CurveSummary curve_summary;
                curve_summary.variant_id = variant.variant_id;
                curve_summary.mode = mode;
                curve_summary.solver_model = solver_model;
                curve_summary.total_points = config.point_count;

                for (int sample_index = 0; sample_index < config.point_count; ++sample_index) {
                    // The plotted ordinate comes from the underlying point solver as
                    // kz normalized against n4; this loop simply samples that curve.
                    const double b_over_a4 =
                        config.b_over_A4_min + step * static_cast<double>(sample_index);
                    const double b = b_over_a4 * a4;
                    const auto point = SolveFigure6Point(config, variant, mode, solver_model, b);

                    if (point.domain_valid) {
                        ++curve_summary.valid_points;
                    }

                    if (point.guided) {
                        ++curve_summary.guided_points;
                    }

                    result.samples.push_back(
                        {config.panel_id, variant.variant_id, mode.curve_id, sample_index, b_over_a4, point}
                    );
                }

                result.curves.push_back(curve_summary);
            }
        }
    }

    return result;
}

}  // namespace marcatili
