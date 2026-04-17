#include "marcatili/io/coupler_io.hpp"

#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

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

}  // namespace

marcatili::CouplerPointConfig ParseCouplerPointConfig(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::CouplerPointConfig config;

    const auto case_id = FindStringValue(json_text, "case_id");
    if (case_id.has_value()) {
        config.case_id = *case_id;
    } else {
        config.case_id =
            FindStringValue(json_text, "case_name").value_or("CP-POINT-UNSPECIFIED");
    }

    const auto article_target = FindStringValue(json_text, "article_target");
    if (article_target.has_value()) {
        config.article_target = *article_target;
    } else {
        config.article_target =
            FindStringValue(json_text, "article_scope").value_or("");
    }

    config.csv_output_path =
        FindStringValue(json_text, "csv_file")
            .value_or(BuildDefaultCsvPath(cli_output_json));

    config.solver_model = marcatili::ParseSingleGuideSolverModel(
        FindStringValue(json_text, "solver_model").value_or("exact")
    );

    config.transverse_equation = marcatili::ParseCouplerTransverseEquation(
        FindStringValue(json_text, "transverse_equation").value_or("eq6")
    );

    config.p = FindIntWithFallback(json_text, "mode_indices.p", "p").value_or(1);

    config.a_over_A5 =
        RequireDoubleWithFallback(json_text, "normalized_geometry.a_over_A5", "a_over_A5");
    config.c_over_a =
        RequireDoubleWithFallback(json_text, "normalized_geometry.c_over_a", "c_over_a");

    const auto index_ratio_squared =
        FindDoubleWithFallback(json_text, "materials.index_ratio_squared", "index_ratio_squared");
    const auto n1_over_n5 =
        FindDoubleWithFallback(json_text, "materials.n1_over_n5", "n1_over_n5");

    if (index_ratio_squared.has_value()) {
        config.index_ratio_squared = *index_ratio_squared;
    } else if (n1_over_n5.has_value() && *n1_over_n5 > 0.0) {
        config.index_ratio_squared = 1.0 / (*n1_over_n5 * *n1_over_n5);
    } else {
        config.index_ratio_squared = 0.0;
    }

    return config;
}

std::string BuildCouplerPointJsonReport(
    const marcatili::CouplerPointResult& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;
    json << "{\n";

    AppendJsonField(json, "app", "\"solve_coupler\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(
        json,
        "model",
        "\"" + EscapeJson(ToString(result.config.solver_model)) + "\""
    );
    AppendJsonField(
        json,
        "transverse_equation",
        "\"" + EscapeJson(ToString(result.config.transverse_equation)) + "\""
    );
    AppendJsonField(json, "equations_used", JsonStringOrNull(result.equations_used));
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(json, "output_csv_file", JsonStringOrNull(result.config.csv_output_path));
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));
    AppendJsonField(json, "domain_valid", result.domain_valid ? "true" : "false");

    json << "  \"normalized_inputs\": {\n";
    AppendJsonField(json, "p", std::to_string(result.config.p), true, 4);
    AppendJsonField(json, "a_over_A5", JsonNumber(result.config.a_over_A5), true, 4);
    AppendJsonField(json, "c_over_a", JsonNumber(result.config.c_over_a), true, 4);
    AppendJsonField(
        json,
        "index_ratio_squared",
        JsonNumberOrNull(result.config.index_ratio_squared),
        false,
        4
    );
    json << "  },\n";

    json << "  \"normalized_outputs\": {\n";
    AppendJsonField(json, "a_over_A5", JsonNumberOrNull(result.a_over_A5), true, 4);
    AppendJsonField(json, "c_over_A5", JsonNumberOrNull(result.c_over_A5), true, 4);
    AppendJsonField(json, "kx_A5_over_pi", JsonNumberOrNull(result.kx_A5_over_pi), true, 4);
    AppendJsonField(
        json,
        "sqrt_one_minus_kx_A5_over_pi_squared",
        JsonNumberOrNull(result.sqrt_one_minus_kx_A5_over_pi_squared),
        true,
        4
    );
    AppendJsonField(
        json,
        "normalized_coupling",
        JsonNumberOrNull(result.normalized_coupling),
        false,
        4
    );
    json << "  },\n";

    AppendJsonField(
        json,
        "note",
        JsonStringOrNull(
            "This executable currently solves the normalized coupling model from Eq. (34). "
            "Computing dimensional |K| and L requires additional dimensional inputs tied to "
            "kz normalization."
        ),
        false
    );

    json << "}\n";
    return json.str();
}

std::string BuildCouplerPointCsvReport(const marcatili::CouplerPointResult& result) {
    std::ostringstream csv;

    csv << "case_id,solver_model,transverse_equation,p,a_over_A5,c_over_a,c_over_A5,"
           "index_ratio_squared,kx_A5_over_pi,sqrt_one_minus_kx_A5_over_pi_squared,"
           "normalized_coupling,domain_valid,status,equations_used\n";

    csv << EscapeCsv(result.config.case_id) << ","
        << EscapeCsv(ToString(result.config.solver_model)) << ","
        << EscapeCsv(ToString(result.config.transverse_equation)) << ","
        << result.config.p << ","
        << CsvNumber(result.config.a_over_A5) << ","
        << CsvNumber(result.config.c_over_a) << ","
        << CsvNumber(result.c_over_A5) << ","
        << CsvNumber(result.config.index_ratio_squared) << ","
        << CsvNumber(result.kx_A5_over_pi) << ","
        << CsvNumber(result.sqrt_one_minus_kx_A5_over_pi_squared) << ","
        << CsvNumber(result.normalized_coupling) << ","
        << (result.domain_valid ? "1" : "0") << ","
        << EscapeCsv(result.status) << ","
        << EscapeCsv(result.equations_used) << "\n";

    return csv.str();
}

}  // namespace marcatili::io
