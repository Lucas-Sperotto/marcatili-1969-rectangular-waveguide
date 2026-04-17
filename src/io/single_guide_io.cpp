#include "marcatili/io/single_guide_io.hpp"

#include <cmath>
#include <iomanip>
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

std::string JsonNumberOrNull(double value) {
    if (!std::isfinite(value)) {
        return "null";
    }

    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
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

int RequireIntWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto dotted_value = FindIntValue(json_text, dotted_key);
    if (dotted_value.has_value()) {
        return *dotted_value;
    }

    return RequireIntValue(json_text, flat_key);
}

double RequireDoubleWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto dotted_value = FindDoubleValue(json_text, dotted_key);
    if (dotted_value.has_value()) {
        return *dotted_value;
    }

    return RequireDoubleValue(json_text, flat_key);
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

marcatili::SingleGuideConfig ParseSingleGuideConfig(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::SingleGuideConfig config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");

    config.csv_output_path =
        FindStringValue(json_text, "csv_file")
            .value_or(BuildDefaultCsvPath(cli_output_json));

    config.solver_model = ParseSingleGuideSolverModel(
        FindStringValue(json_text, "solver_model").value_or("closed_form")
    );

    config.family =
        marcatili::ParseSingleGuideFamily(RequireStringValue(json_text, "mode_family"));

    config.p = RequireIntWithFallback(json_text, "mode_indices.p", "p");
    config.q = RequireIntWithFallback(json_text, "mode_indices.q", "q");

    config.wavelength =
        RequireDoubleWithFallback(json_text, "geometry.wavelength", "wavelength");
    config.a =
        RequireDoubleWithFallback(json_text, "geometry.a", "a");
    config.b =
        RequireDoubleWithFallback(json_text, "geometry.b", "b");

    config.n1 =
        RequireDoubleWithFallback(json_text, "materials.n1", "n1");
    config.n2 =
        RequireDoubleWithFallback(json_text, "materials.n2", "n2");
    config.n3 =
        RequireDoubleWithFallback(json_text, "materials.n3", "n3");
    config.n4 =
        RequireDoubleWithFallback(json_text, "materials.n4", "n4");
    config.n5 =
        RequireDoubleWithFallback(json_text, "materials.n5", "n5");

    return config;
}

std::string BuildSingleGuideJsonReport(
    const marcatili::SingleGuideResult& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;
    json << "{\n";

    AppendJsonField(json, "app", "\"solve_single_guide\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(
        json,
        "model",
        "\"" + EscapeJson(ToString(result.config.solver_model)) + "\""
    );
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(json, "output_csv_file", JsonStringOrNull(result.config.csv_output_path));
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));
    AppendJsonField(
        json,
        "solver_model",
        "\"" + EscapeJson(ToString(result.config.solver_model)) + "\""
    );
    AppendJsonField(
        json,
        "mode_family",
        "\"" + EscapeJson(ToString(result.config.family)) + "\""
    );

    json << "  \"mode_indices\": {\n";
    AppendJsonField(json, "p", std::to_string(result.config.p), true, 4);
    AppendJsonField(json, "q", std::to_string(result.config.q), false, 4);
    json << "  },\n";

    AppendJsonField(json, "guided", result.guided ? "true" : "false");
    AppendJsonField(json, "domain_valid", result.domain_valid ? "true" : "false");
    AppendJsonField(
        json,
        "equations_used",
        "\"" + EscapeJson(result.equations_used) + "\""
    );

    json << "  \"geometry\": {\n";
    AppendJsonField(json, "wavelength", JsonNumberOrNull(result.config.wavelength), true, 4);
    AppendJsonField(json, "a", JsonNumberOrNull(result.config.a), true, 4);
    AppendJsonField(json, "b", JsonNumberOrNull(result.config.b), false, 4);
    json << "  },\n";

    json << "  \"materials\": {\n";
    AppendJsonField(json, "n1", JsonNumberOrNull(result.config.n1), true, 4);
    AppendJsonField(json, "n2", JsonNumberOrNull(result.config.n2), true, 4);
    AppendJsonField(json, "n3", JsonNumberOrNull(result.config.n3), true, 4);
    AppendJsonField(json, "n4", JsonNumberOrNull(result.config.n4), true, 4);
    AppendJsonField(json, "n5", JsonNumberOrNull(result.config.n5), false, 4);
    json << "  },\n";

    json << "  \"derived\": {\n";
    AppendJsonField(json, "k0", JsonNumberOrNull(result.k0), true, 4);
    AppendJsonField(json, "k1", JsonNumberOrNull(result.k1), true, 4);
    AppendJsonField(json, "k2", JsonNumberOrNull(result.k2), true, 4);
    AppendJsonField(json, "k3", JsonNumberOrNull(result.k3), true, 4);
    AppendJsonField(json, "k4", JsonNumberOrNull(result.k4), true, 4);
    AppendJsonField(json, "k5", JsonNumberOrNull(result.k5), true, 4);
    AppendJsonField(json, "A2", JsonNumberOrNull(result.A2), true, 4);
    AppendJsonField(json, "A3", JsonNumberOrNull(result.A3), true, 4);
    AppendJsonField(json, "A4", JsonNumberOrNull(result.A4), true, 4);
    AppendJsonField(json, "A5", JsonNumberOrNull(result.A5), true, 4);
    AppendJsonField(json, "kx", JsonNumberOrNull(result.kx), true, 4);
    AppendJsonField(json, "ky", JsonNumberOrNull(result.ky), true, 4);
    AppendJsonField(json, "kz", JsonNumberOrNull(result.kz), true, 4);
    AppendJsonField(json, "xi3", JsonNumberOrNull(result.xi3), true, 4);
    AppendJsonField(json, "xi5", JsonNumberOrNull(result.xi5), true, 4);
    AppendJsonField(json, "eta2", JsonNumberOrNull(result.eta2), true, 4);
    AppendJsonField(json, "eta4", JsonNumberOrNull(result.eta4), true, 4);
    AppendJsonField(json, "b_over_A4", JsonNumberOrNull(result.b_over_A4), true, 4);
    AppendJsonField(
        json,
        "kz_normalized_against_n4",
        JsonNumberOrNull(result.kz_normalized_against_n4),
        false,
        4
    );
    json << "  },\n";

    json << "  \"approximation_checks\": {\n";
    AppendJsonField(
        json,
        "kx_A3_over_pi_squared",
        JsonNumberOrNull(result.approximation_checks.kx_a3_over_pi_squared),
        true,
        4
    );
    AppendJsonField(
        json,
        "kx_A5_over_pi_squared",
        JsonNumberOrNull(result.approximation_checks.kx_a5_over_pi_squared),
        true,
        4
    );
    AppendJsonField(
        json,
        "ky_A2_over_pi_squared",
        JsonNumberOrNull(result.approximation_checks.ky_a2_over_pi_squared),
        true,
        4
    );
    AppendJsonField(
        json,
        "ky_A4_over_pi_squared",
        JsonNumberOrNull(result.approximation_checks.ky_a4_over_pi_squared),
        false,
        4
    );
    json << "  }\n";

    json << "}\n";
    return json.str();
}

std::string BuildSingleGuideCsvReport(const marcatili::SingleGuideResult& result) {
    std::ostringstream csv;

    csv << "case_id,solver_model,mode_family,p,q,wavelength,a,b,n1,n2,n3,n4,n5,"
           "guided,domain_valid,k0,k1,k2,k3,k4,k5,"
           "A2,A3,A4,A5,kx,ky,kz,xi3,xi5,eta2,eta4,"
           "b_over_A4,kz_normalized_against_n4,"
           "kx_A3_over_pi_squared,kx_A5_over_pi_squared,"
           "ky_A2_over_pi_squared,ky_A4_over_pi_squared\n";

    csv << EscapeCsv(result.config.case_id) << ","
        << EscapeCsv(ToString(result.config.solver_model)) << ","
        << EscapeCsv(ToString(result.config.family)) << ","
        << result.config.p << ","
        << result.config.q << ","
        << CsvNumber(result.config.wavelength) << ","
        << CsvNumber(result.config.a) << ","
        << CsvNumber(result.config.b) << ","
        << CsvNumber(result.config.n1) << ","
        << CsvNumber(result.config.n2) << ","
        << CsvNumber(result.config.n3) << ","
        << CsvNumber(result.config.n4) << ","
        << CsvNumber(result.config.n5) << ","
        << (result.guided ? "1" : "0") << ","
        << (result.domain_valid ? "1" : "0") << ","
        << CsvNumber(result.k0) << ","
        << CsvNumber(result.k1) << ","
        << CsvNumber(result.k2) << ","
        << CsvNumber(result.k3) << ","
        << CsvNumber(result.k4) << ","
        << CsvNumber(result.k5) << ","
        << CsvNumber(result.A2) << ","
        << CsvNumber(result.A3) << ","
        << CsvNumber(result.A4) << ","
        << CsvNumber(result.A5) << ","
        << CsvNumber(result.kx) << ","
        << CsvNumber(result.ky) << ","
        << CsvNumber(result.kz) << ","
        << CsvNumber(result.xi3) << ","
        << CsvNumber(result.xi5) << ","
        << CsvNumber(result.eta2) << ","
        << CsvNumber(result.eta4) << ","
        << CsvNumber(result.b_over_A4) << ","
        << CsvNumber(result.kz_normalized_against_n4) << ","
        << CsvNumber(result.approximation_checks.kx_a3_over_pi_squared) << ","
        << CsvNumber(result.approximation_checks.kx_a5_over_pi_squared) << ","
        << CsvNumber(result.approximation_checks.ky_a2_over_pi_squared) << ","
        << CsvNumber(result.approximation_checks.ky_a4_over_pi_squared) << "\n";

    return csv.str();
}

}  // namespace marcatili::io
