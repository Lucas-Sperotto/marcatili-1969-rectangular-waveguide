#include "marcatili/io/fig8_io.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>

#include "marcatili/io/schema_json.hpp"
#include "marcatili/io/text_io.hpp"

namespace marcatili::io {
namespace {

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
    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
}

std::string DefaultCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".csv");
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
        FindStringValue(json_text, "csv_file").value_or(DefaultCsvPath(cli_output_json));
    config.wavelength = RequireDoubleValue(json_text, "wavelength");
    config.a_over_b = RequireDoubleValue(json_text, "a_over_b");
    config.n1 = RequireDoubleValue(json_text, "n1");
    config.n4 = RequireDoubleValue(json_text, "n4");
    config.a_over_A_min = RequireDoubleValue(json_text, "a_over_A_min");
    config.a_over_A_max = RequireDoubleValue(json_text, "a_over_A_max");
    config.point_count = RequireIntValue(json_text, "point_count");

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
    constexpr double kArticleSingleModeUpperBound = 1.4;

    std::vector<std::string> exact_guided_curve_ids_below_bound;
    for (const auto& sample : result.samples) {
        if (sample.point.config.solver_model != marcatili::SingleGuideSolverModel::kExact) {
            continue;
        }

        if (!sample.point.guided || sample.a_over_A > kArticleSingleModeUpperBound) {
            continue;
        }

        if (std::find(
                exact_guided_curve_ids_below_bound.begin(),
                exact_guided_curve_ids_below_bound.end(),
                sample.curve_id
            ) == exact_guided_curve_ids_below_bound.end()) {
            exact_guided_curve_ids_below_bound.push_back(sample.curve_id);
        }
    }

    std::ostringstream json;

    json << "{\n";
    json << "  \"app\": \"reproduce_fig8\",\n";
    json << "  \"status\": \"" << EscapeJson(result.status) << "\",\n";
    json << "  \"input_file\": " << JsonStringOrNull(input_file) << ",\n";
    json << "  \"output_json_file\": " << JsonStringOrNull(output_json_file) << ",\n";
    json << "  \"output_csv_file\": " << JsonStringOrNull(result.config.csv_output_path) << ",\n";
    json << "  \"case_id\": \"" << EscapeJson(result.config.case_id) << "\",\n";
    json << "  \"article_target\": " << JsonStringOrNull(result.config.article_target) << ",\n";
    json << "  \"ocr_note\": " << JsonStringOrNull(result.config.ocr_note) << ",\n";
    json << "  \"model_note\": "
         << JsonStringOrNull(
                "Top interface treated as a PEC wall; y-characteristic equation "
                "modeled as the Fig. 4/Appendix A limit with a fixed pi/2 phase shift."
            ) << ",\n";
    json << "  \"sweep\": {\n";
    json << "    \"wavelength\": " << JsonNumber(result.config.wavelength) << ",\n";
    json << "    \"a_over_b\": " << JsonNumber(result.config.a_over_b) << ",\n";
    json << "    \"a_over_A_min\": " << JsonNumber(result.config.a_over_A_min) << ",\n";
    json << "    \"a_over_A_max\": " << JsonNumber(result.config.a_over_A_max) << ",\n";
    json << "    \"point_count\": " << result.config.point_count << "\n";
    json << "  },\n";
    json << "  \"materials\": {\n";
    json << "    \"n1\": " << JsonNumber(result.config.n1) << ",\n";
    json << "    \"n4\": " << JsonNumber(result.config.n4) << "\n";
    json << "  },\n";
    json << "  \"article_single_mode_check\": {\n";
    json << "    \"a_over_A_upper_bound\": " << JsonNumber(kArticleSingleModeUpperBound) << ",\n";
    json << "    \"expected_guided_curve_id\": \"E_y_1_1\",\n";
    json << "    \"condition_met\": "
         << ((exact_guided_curve_ids_below_bound.size() == 1 &&
              exact_guided_curve_ids_below_bound.front() == "E_y_1_1")
                 ? "true"
                 : "false")
         << ",\n";
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
        json << "      \"curve_id\": \"" << EscapeJson(curve.mode.curve_id) << "\",\n";
        json << "      \"solver_model\": \"" << EscapeJson(ToString(curve.solver_model)) << "\",\n";
        json << "      \"mode_family\": \"" << EscapeJson(ToString(curve.mode.family)) << "\",\n";
        json << "      \"p\": " << curve.mode.p << ",\n";
        json << "      \"q\": " << curve.mode.q << ",\n";
        json << "      \"total_points\": " << curve.total_points << ",\n";
        json << "      \"valid_points\": " << curve.valid_points << ",\n";
        json << "      \"guided_points\": " << curve.guided_points << "\n";
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
        csv << result.config.case_id << ","
            << sample.curve_id << ","
            << ToString(sample.point.config.solver_model) << ","
            << ToString(sample.point.config.family) << ","
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
