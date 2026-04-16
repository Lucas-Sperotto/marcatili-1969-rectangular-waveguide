#include "marcatili/physics/table1.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace marcatili {
namespace {

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

double ComputeA(double wavelength, double n1, double nv) {
    return wavelength / (2.0 * std::sqrt(n1 * n1 - nv * nv));
}

void ValidateConfig(const Table1Config& config) {
    if (config.wavelength <= 0.0) {
        throw std::runtime_error("wavelength must be positive.");
    }

    if (config.n1 <= 0.0) {
        throw std::runtime_error("n1 must be positive.");
    }

    if (config.rows.empty()) {
        throw std::runtime_error("At least one row must be listed in the rows array.");
    }

    if (config.solver_models.empty()) {
        throw std::runtime_error("At least one solver model must be listed.");
    }

    if (config.search.max_p < 1 || config.search.max_q < 1) {
        throw std::runtime_error("search.max_p and search.max_q must be positive.");
    }

    if (config.search.b_normalized_min <= 0.0 ||
        config.search.b_normalized_max <= config.search.b_normalized_min) {
        throw std::runtime_error(
            "search.b_normalized_min and search.b_normalized_max must define a positive interval."
        );
    }

    if (config.search.cutoff_tolerance <= 0.0) {
        throw std::runtime_error("search.cutoff_tolerance must be positive.");
    }

    for (const auto& row : config.rows) {
        if (row.row_id.empty()) {
            throw std::runtime_error("rows entries must provide a non-empty row_id.");
        }

        if (row.a_over_b <= 0.0) {
            throw std::runtime_error("rows entries must provide a positive a_over_b.");
        }

        if (row.article_dimension_normalized <= 0.0) {
            throw std::runtime_error(
                "rows entries must provide a positive article_dimension_normalized."
            );
        }

        const double external_max = std::max(std::max(row.n2, row.n3), std::max(row.n4, row.n5));
        if (config.n1 <= external_max) {
            throw std::runtime_error("Table I rows require n1 > n2, n3, n4, n5.");
        }
    }
}

bool EvaluateGuidance(
    const Table1Config& config,
    const Table1RowSpec& row,
    SingleGuideSolverModel solver_model,
    SingleGuideFamily family,
    int p,
    int q,
    double b_normalized,
    SingleGuideResult* result
) {
    SingleGuideConfig point_config;
    point_config.case_id = config.case_id + "_" + row.row_id;
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = solver_model;
    point_config.family = family;
    point_config.p = p;
    point_config.q = q;
    point_config.wavelength = config.wavelength;
    point_config.b = b_normalized * config.wavelength / config.n1;
    point_config.a = row.a_over_b * point_config.b;
    point_config.n1 = config.n1;
    point_config.n2 = row.n2;
    point_config.n3 = row.n3;
    point_config.n4 = row.n4;
    point_config.n5 = row.n5;

    SingleGuideResult local_result = SolveSingleGuide(point_config);
    const bool guided = local_result.guided;

    if (result != nullptr) {
        *result = local_result;
    }

    return guided;
}

Table1ModeCutoff FindModeCutoff(
    const Table1Config& config,
    const Table1RowSpec& row,
    SingleGuideSolverModel solver_model,
    SingleGuideFamily family,
    int p,
    int q
) {
    Table1ModeCutoff cutoff;
    cutoff.row_id = row.row_id;
    cutoff.article_panel_id = row.article_panel_id;
    cutoff.solver_model = solver_model;
    cutoff.family = family;
    cutoff.p = p;
    cutoff.q = q;
    cutoff.mode_id = BuildTable1ModeId(family, p, q);

    const double lower = config.search.b_normalized_min;
    const double upper = config.search.b_normalized_max;

    if (EvaluateGuidance(config, row, solver_model, family, p, q, lower, nullptr)) {
        cutoff.cutoff_found = true;
        cutoff.cutoff_b_normalized = lower;
    } else if (!EvaluateGuidance(config, row, solver_model, family, p, q, upper, nullptr)) {
        cutoff.cutoff_found = false;
        cutoff.cutoff_b_normalized = NaN();
        cutoff.cutoff_a_normalized = NaN();
        cutoff.cutoff_b_over_A4 = NaN();
        return cutoff;
    } else {
        double left = lower;
        double right = upper;

        for (int iteration = 0; iteration < 100; ++iteration) {
            const double midpoint = 0.5 * (left + right);
            if (EvaluateGuidance(config, row, solver_model, family, p, q, midpoint, nullptr)) {
                right = midpoint;
            } else {
                left = midpoint;
            }

            if (std::abs(right - left) <= config.search.cutoff_tolerance) {
                break;
            }
        }

        cutoff.cutoff_found = true;
        cutoff.cutoff_b_normalized = right;
    }

    cutoff.cutoff_a_normalized = row.a_over_b * cutoff.cutoff_b_normalized;
    cutoff.cutoff_b_over_A4 =
        (cutoff.cutoff_b_normalized * config.wavelength / config.n1) /
        ComputeA(config.wavelength, config.n1, row.n4);
    return cutoff;
}

}  // namespace

std::string BuildTable1ModeId(SingleGuideFamily family, int p, int q) {
    return ToString(family) + "_" + std::to_string(p) + "_" + std::to_string(q);
}

Table1Result SolveTable1(const Table1Config& config) {
    ValidateConfig(config);

    Table1Result result;
    result.config = config;
    result.status = "ok";

    for (const auto& row : config.rows) {
        for (const auto& solver_model : config.solver_models) {
            Table1RowSummary summary;
            summary.row_id = row.row_id;
            summary.article_panel_id = row.article_panel_id;
            summary.solver_model = solver_model;
            summary.a_over_b = row.a_over_b;
            summary.n2 = row.n2;
            summary.n3 = row.n3;
            summary.n4 = row.n4;
            summary.n5 = row.n5;
            summary.article_dimension_normalized = row.article_dimension_normalized;
            summary.computed_dimension_normalized = NaN();
            summary.computed_b_normalized = NaN();
            summary.computed_a_normalized = NaN();
            summary.computed_b_over_A4 = NaN();
            summary.absolute_error = NaN();
            summary.relative_error = NaN();

            double best_cutoff = std::numeric_limits<double>::infinity();
            Table1ModeCutoff best_mode;

            for (int p = 1; p <= config.search.max_p; ++p) {
                for (int q = 1; q <= config.search.max_q; ++q) {
                    if (p == 1 && q == 1) {
                        continue;
                    }

                    for (const auto family : {SingleGuideFamily::kEy, SingleGuideFamily::kEx}) {
                        const auto cutoff = FindModeCutoff(config, row, solver_model, family, p, q);
                        result.mode_cutoffs.push_back(cutoff);

                        if (cutoff.cutoff_found && cutoff.cutoff_b_normalized < best_cutoff) {
                            best_cutoff = cutoff.cutoff_b_normalized;
                            best_mode = cutoff;
                        }
                    }
                }
            }

            if (std::isfinite(best_cutoff)) {
                summary.limiting_cutoff_found = true;
                summary.limiting_mode_id = best_mode.mode_id;
                summary.limiting_family = best_mode.family;
                summary.limiting_p = best_mode.p;
                summary.limiting_q = best_mode.q;
                summary.computed_b_normalized = best_mode.cutoff_b_normalized;
                summary.computed_a_normalized = best_mode.cutoff_a_normalized;
                summary.computed_dimension_normalized = best_mode.cutoff_a_normalized;
                summary.computed_b_over_A4 = best_mode.cutoff_b_over_A4;
                summary.absolute_error =
                    summary.computed_dimension_normalized - summary.article_dimension_normalized;
                summary.relative_error =
                    summary.absolute_error / summary.article_dimension_normalized;

                const double probe_b_normalized =
                    std::max(
                        config.search.b_normalized_min,
                        best_mode.cutoff_b_normalized - config.search.cutoff_tolerance
                    );

                summary.ey11_guided_just_below_cutoff = EvaluateGuidance(
                    config,
                    row,
                    solver_model,
                    SingleGuideFamily::kEy,
                    1,
                    1,
                    probe_b_normalized,
                    nullptr
                );
                summary.ex11_guided_just_below_cutoff = EvaluateGuidance(
                    config,
                    row,
                    solver_model,
                    SingleGuideFamily::kEx,
                    1,
                    1,
                    probe_b_normalized,
                    nullptr
                );
            }

            result.row_summaries.push_back(summary);
        }
    }

    return result;
}

}  // namespace marcatili
