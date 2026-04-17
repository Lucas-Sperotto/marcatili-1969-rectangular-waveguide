#include "marcatili/io/coupler_io.hpp"

#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>

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

std::string DefaultCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".csv");
}

marcatili::SingleGuideSolverModel ParseCouplerSolverModel(const std::string& solver_model_text) {
    if (solver_model_text == "closed_form" || solver_model_text == "approx" ||
        solver_model_text == "approximate") {
        return marcatili::SingleGuideSolverModel::kClosedForm;
    }

    if (solver_model_text == "exact" || solver_model_text == "transcendental") {
        return marcatili::SingleGuideSolverModel::kExact;
    }

    throw std::runtime_error(
        "Unsupported solver_model. Use one of: closed_form, exact."
    );
}

}  // namespace

marcatili::CouplerPointConfig ParseCouplerPointConfig(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::CouplerPointConfig config;

    const auto find_double_any =
        [&](const std::string& dotted_key, const std::string& flat_key) -> std::optional<double> {
        const auto dotted_value = FindDoubleValue(json_text, dotted_key);
        if (dotted_value.has_value()) {
            return dotted_value;
        }

        return FindDoubleValue(json_text, flat_key);
    };

    const auto require_double_any = [&](const std::string& dotted_key, const std::string& flat_key) {
        const auto value = find_double_any(dotted_key, flat_key);
        if (!value.has_value()) {
            throw std::runtime_error(
                "Missing required numeric key: " + dotted_key + " (or " + flat_key + ")"
            );
        }

        return *value;
    };

    const auto find_int_any =
        [&](const std::string& dotted_key, const std::string& flat_key) -> std::optional<int> {
        const auto dotted_value = FindIntValue(json_text, dotted_key);
        if (dotted_value.has_value()) {
            return dotted_value;
        }

        return FindIntValue(json_text, flat_key);
    };

    const auto case_id = FindStringValue(json_text, "case_id");
    if (case_id.has_value()) {
        config.case_id = *case_id;
    } else {
        config.case_id = FindStringValue(json_text, "case_name").value_or("CP-POINT-UNSPECIFIED");
    }

    const auto article_target = FindStringValue(json_text, "article_target");
    if (article_target.has_value()) {
        config.article_target = *article_target;
    } else {
        config.article_target = FindStringValue(json_text, "article_scope").value_or("");
    }
    config.csv_output_path =
        FindStringValue(json_text, "csv_file").value_or(DefaultCsvPath(cli_output_json));

    config.solver_model = ParseCouplerSolverModel(
        FindStringValue(json_text, "solver_model").value_or("exact")
    );

    config.transverse_equation = marcatili::ParseCouplerTransverseEquation(
        FindStringValue(json_text, "transverse_equation").value_or("eq6")
    );

    config.p = find_int_any("mode_indices.p", "p").value_or(1);
    config.a_over_A5 = require_double_any("normalized_geometry.a_over_A5", "a_over_A5");
    config.c_over_a = require_double_any("normalized_geometry.c_over_a", "c_over_a");

    const auto index_ratio_squared =
        find_double_any("materials.index_ratio_squared", "index_ratio_squared");
    const auto n1_over_n5 = find_double_any("materials.n1_over_n5", "n1_over_n5");

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
    json << "  \"app\": \"solve_coupler\",\n";
    json << "  \"status\": \"" << EscapeJson(result.status) << "\",\n";
    json << "  \"model\": \"" << EscapeJson(ToString(result.config.solver_model)) << "\",\n";
    json << "  \"transverse_equation\": \""
         << EscapeJson(ToString(result.config.transverse_equation)) << "\",\n";
    json << "  \"equations_used\": " << JsonStringOrNull(result.equations_used) << ",\n";
    json << "  \"input_file\": " << JsonStringOrNull(input_file) << ",\n";
    json << "  \"output_json_file\": " << JsonStringOrNull(output_json_file) << ",\n";
    json << "  \"output_csv_file\": " << JsonStringOrNull(result.config.csv_output_path) << ",\n";
    json << "  \"case_id\": \"" << EscapeJson(result.config.case_id) << "\",\n";
    json << "  \"article_target\": " << JsonStringOrNull(result.config.article_target) << ",\n";
    json << "  \"domain_valid\": " << (result.domain_valid ? "true" : "false") << ",\n";
    json << "  \"normalized_inputs\": {\n";
    json << "    \"p\": " << result.config.p << ",\n";
    json << "    \"a_over_A5\": " << JsonNumber(result.a_over_A5) << ",\n";
    json << "    \"c_over_a\": " << JsonNumber(result.config.c_over_a) << ",\n";
    json << "    \"index_ratio_squared\": "
         << JsonNumberOrNull(result.config.index_ratio_squared) << "\n";
    json << "  },\n";
    json << "  \"normalized_outputs\": {\n";
    json << "    \"c_over_A5\": " << JsonNumberOrNull(result.c_over_A5) << ",\n";
    json << "    \"kx_A5_over_pi\": " << JsonNumberOrNull(result.kx_A5_over_pi) << ",\n";
    json << "    \"sqrt_one_minus_kx_A5_over_pi_squared\": "
         << JsonNumberOrNull(result.sqrt_one_minus_kx_A5_over_pi_squared) << ",\n";
    json << "    \"normalized_coupling\": "
         << JsonNumberOrNull(result.normalized_coupling) << "\n";
    json << "  },\n";
    json << "  \"note\": "
         << JsonStringOrNull(
                "This executable currently solves the normalized coupling model from Eq. (34). "
                "Computing dimensional |K| and L requires additional dimensional inputs tied to "
                "kz normalization."
            ) << "\n";
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
        << CsvNumber(result.a_over_A5) << ","
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
