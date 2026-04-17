#include "marcatili/io/fig8_io.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "marcatili/io/schema_json.hpp"
#include "marcatili/io/text_io.hpp"

namespace marcatili::io {
namespace {

constexpr double kArticleSingleModeUpperBound = 1.4;
constexpr const char* kExpectedSingleModeCurveId = "E_y_1_1";

std::string JsonStringOrNull(const std::string& value) {
    if (value.empty()) {
        return "null";
    }

    return "\"" + EscapeJson(value) + "\"";
}

std::string JsonNumber(double value) {
    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
}

std::string JsonNumberOrNull(double value) {
    if (!std::isfinite(value)) {
        return "null";
    }

    return JsonNumber(value);
}

std::string CsvNumber(double value) {
    if (!std::isfinite(value)) {
        return "nan";
    }

    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
}

std::string BuildDefaultCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".csv");
}

std::optional<double> FindDoubleWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto dotted_value = FindDoubleValue(json_text, dotted_key);
    if (dotted_value.has_value()) {
        return dotted_value;
    }

    return FindDoubleValue(json_text, flat_key);
}

double RequireDoubleWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto value = FindDoubleWithFallback(json_text, dotted_key, flat_key);
    if (!value.has_value()) {
        throw std::runtime_error(
            "Missing required numeric key: " + dotted_key + " (or " + flat_key + ")"
        );
    }

    return *value;
}

std::optional<int> FindIntWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto dotted_value = FindIntValue(json_text, dotted_key);
    if (dotted_value.has_value()) {
        return dotted_value;
    }

    return FindIntValue(json_text, flat_key);
}

int RequireIntWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto value = FindIntWithFallback(json_text, dotted_key, flat_key);
    if (!value.has_value()) {
        throw std::runtime_error(
            "Missing required integer key: " + dotted_key + " (or " + flat_key + ")"
        );
    }

    return *value;
}

void AppendJsonField(
    std::ostringstream& json,
    const std::string& key,
    const std::string& raw_value,
    bool trailing_comma = true,
    int indent = 2
) {
    json << std::string(indent, ' ')
         << "\"" << key << "\": " << raw_value;

    if (trailing_comma) {
        json << ",";
    }

    json << "\n";
}

std::vector<std::string> CollectExactGuidedCurveIdsBelowBound(
    const marcatili::Figure8Result& result,
    double a_over_A_upper_bound
) {
    std::vector<std::string> curve_ids;

    for (const auto& sample : result.samples) {
        if (sample.point.config.solver_model != marcatili::SingleGuideSolverModel::kExact) {
            continue;
        }

        if (!sample.point.guided) {
            continue;
        }

        if (sample.a_over_A > a_over_A_upper_bound) {
            continue;
        }

        if (std::find(curve_ids.begin(), curve_ids.end(), sample.curve_id) == curve_ids.end()) {
            curve_ids.push_back(sample.curve_id);
        }
    }

    return curve_ids;
}

bool ArticleSingleModeConditionMet(const std::vector<std::string>& guided_curve_ids) {
    return guided_curve_ids.size() == 1 &&
           guided_curve_ids.front() == kExpectedSingleModeCurveId;
}

}  // namespace

marcatili::Figure8Config ParseFigure8Config(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::Figure8Config config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");
    config.ocr_note = FindStringValue(json_text, "ocr_note").value_or("");

    config.csv_output_path =
        FindStringValue(json_text, "csv_file")
            .value_or(BuildDefaultCsvPath(cli_output_json));

    config.wavelength =
        RequireDoubleWithFallback(json_text, "geometry.wavelength", "wavelength");
    config.a_over_b =
        RequireDoubleWithFallback(json_text, "geometry.a_over_b", "a_over_b");

    config.n1 =
        RequireDoubleWithFallback(json_text, "materials.n1", "n1");
    config.n4 =
        RequireDoubleWithFallback(json_text, "materials.n4", "n4");

    config.a_over_A_min =
        RequireDoubleWithFallback(json_text, "sweep.a_over_A_min", "a_over_A_min");
    config.a_over_A_max =
        RequireDoubleWithFallback(json_text, "sweep.a_over_A_max", "a_over_A_max");
    config.point_count =
        RequireIntWithFallback(json_text, "sweep.point_count", "point_count");

    const auto solver_model_texts = FindStringArrayValues(json_text, "solver_models");
    if (solver_model_texts.empty()) {
        config.solver_models.push_back(marcatili::SingleGuideSolverModel::kClosedForm);
    } else {
        for (const auto& solver_model_text : solver_model_texts) {
            config.solver_models.push_back(
                marcatili::ParseSingleGuideSolverModel(solver_model_text)
            );
        }
    }

    for (const auto& mode_text : RequireStringArrayValues(json_text, "modes")) {
        config.modes.push_back(marcatili::ParseFigure8ModeSpec(mode_text));
    }

    return config;
}

std::string BuildFigure8JsonReport(
    const marcatili::Figure8Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    const std::vector<std::string> exact_guided_curve_ids_below_bound =
        CollectExactGuidedCurveIdsBelowBound(result, kArticleSingleModeUpperBound);

    std::ostringstream json;
    json << "{\n";

    AppendJsonField(json, "app", "\"reproduce_fig8\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(json, "output_csv_file", JsonStringOrNull(result.config.csv_output_path));
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));
    AppendJsonField(json, "ocr_note", JsonStringOrNull(result.config.ocr_note));
    AppendJsonField(
        json,
        "model_note",
        JsonStringOrNull(
            "Top interface treated as a PEC wall. For E_y, the upper y-boundary is modeled "
            "with a fixed pi/2 phase term; for E_x, the tangential electric field is forced "
            "to vanish on the metal wall and only the lower dielectric phase term is kept."
        )
    );

    json << "  \"sweep\": {\n";
    AppendJsonField(json, "wavelength", JsonNumber(result.config.wavelength), true, 4);
    AppendJsonField(json, "a_over_b", JsonNumber(result.config.a_over_b), true, 4);
    AppendJsonField(json, "a_over_A_min", JsonNumber(result.config.a_over_A_min), true, 4);
    AppendJsonField(json, "a_over_A_max", JsonNumber(result.config.a_over_A_max), true, 4);
    AppendJsonField(json, "point_count", std::to_string(result.config.point_count), false, 4);
    json << "  },\n";

    json << "  \"materials\": {\n";
    AppendJsonField(json, "n1", JsonNumber(result.config.n1), true, 4);
    AppendJsonField(json, "n4", JsonNumber(result.config.n4), false, 4);
    json << "  },\n";

    json << "  \"article_single_mode_check\": {\n";
    AppendJsonField(
        json,
        "a_over_A_upper_bound",
        JsonNumber(kArticleSingleModeUpperBound),
        true,
        4
    );
    AppendJsonField(
        json,
        "expected_guided_curve_id",
        "\"" + EscapeJson(kExpectedSingleModeCurveId) + "\"",
        true,
        4
    );
    AppendJsonField(
        json,
        "condition_met",
        ArticleSingleModeConditionMet(exact_guided_curve_ids_below_bound) ? "true" : "false",
        true,
        4
    );

    json << "    \"guided_curve_ids\": [";
    for (std::size_t index = 0; index < exact_guided_curve_ids_below_bound.size(); ++index) {
        if (index != 0) {
            json << ", ";
        }
        json << "\"" << EscapeJson(exact_guided_curve_ids_below_bound[index]) << "\"";
    }
    json << "]\n";
    json << "  },\n";

    json << "  \"solver_models\": [\n";
    for (std::size_t index = 0; index < result.config.solver_models.size(); ++index) {
        json << "    \"" << EscapeJson(ToString(result.config.solver_models[index])) << "\"";
        if (index + 1 != result.config.solver_models.size()) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";

    json << "  \"curve_summaries\": [\n";
    for (std::size_t index = 0; index < result.curves.size(); ++index) {
        const auto& curve = result.curves[index];

        json << "    {\n";
        AppendJsonField(json, "curve_id", "\"" + EscapeJson(curve.mode.curve_id) + "\"", true, 6);
        AppendJsonField(
            json,
            "solver_model",
            "\"" + EscapeJson(ToString(curve.solver_model)) + "\"",
            true,
            6
        );
        AppendJsonField(
            json,
            "mode_family",
            "\"" + EscapeJson(ToString(curve.mode.family)) + "\"",
            true,
            6
        );
        AppendJsonField(json, "p", std::to_string(curve.mode.p), true, 6);
        AppendJsonField(json, "q", std::to_string(curve.mode.q), true, 6);
        AppendJsonField(json, "total_points", std::to_string(curve.total_points), true, 6);
        AppendJsonField(json, "valid_points", std::to_string(curve.valid_points), true, 6);
        AppendJsonField(json, "guided_points", std::to_string(curve.guided_points), false, 6);
        json << "    }";

        if (index + 1 != result.curves.size()) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n";

    json << "}\n";
    return json.str();
}

std::string BuildFigure8CsvReport(const marcatili::Figure8Result& result) {
    std::ostringstream csv;

    csv << "case_id,curve_id,solver_model,mode_family,p,q,sample_index,a_over_A,a,b,"
           "guided,domain_valid,kx,ky,kz,kz_normalized_against_n4\n";

    for (const auto& sample : result.samples) {
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(sample.curve_id) << ","
            << EscapeCsv(ToString(sample.point.config.solver_model)) << ","
            << EscapeCsv(ToString(sample.point.config.family)) << ","
            << sample.point.config.p << ","
            << sample.point.config.q << ","
            << sample.sample_index << ","
            << CsvNumber(sample.a_over_A) << ","
            << CsvNumber(sample.point.config.a) << ","
            << CsvNumber(sample.point.config.b) << ","
            << (sample.point.guided ? "1" : "0") << ","
            << (sample.point.domain_valid ? "1" : "0") << ","
            << CsvNumber(sample.point.kx) << ","
            << CsvNumber(sample.point.ky) << ","
            << CsvNumber(sample.point.kz) << ","
            << CsvNumber(sample.point.kz_normalized_against_n4) << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
