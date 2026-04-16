#include "marcatili/io/fig7_io.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "marcatili/io/schema_json.hpp"
#include "marcatili/io/text_io.hpp"

namespace marcatili::io {
namespace {

double NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

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

std::string DefaultLinesCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".lines.csv");
}

std::string DefaultIntersectionsCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".intersections.csv");
}

double ParseStringAsDouble(const std::string& text, const std::string& field_name) {
    try {
        return std::stod(text);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid numeric string in " + field_name + ": " + text);
    }
}

}  // namespace

marcatili::Figure7Config ParseFigure7Config(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::Figure7Config config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");
    config.lines_csv_output_path =
        FindStringValue(json_text, "csv_lines_file").value_or(DefaultLinesCsvPath(cli_output_json));
    config.intersections_csv_output_path =
        FindStringValue(json_text, "csv_intersections_file")
            .value_or(DefaultIntersectionsCsvPath(cli_output_json));
    if (const auto article_reference_mode = FindStringValue(json_text, "article_reference_mode")) {
        config.article_reference_mode_line_id =
            marcatili::ParseFigure7ModeSpec(*article_reference_mode).line_id;
    }
    config.article_reference_note =
        FindStringValue(json_text, "article_reference_note").value_or("");
    config.wavelength = RequireDoubleValue(json_text, "wavelength");
    config.a = RequireDoubleValue(json_text, "a");
    config.b = RequireDoubleValue(json_text, "b");
    config.n1 = RequireDoubleValue(json_text, "n1");
    config.n2 = RequireDoubleValue(json_text, "n2");
    config.n3 = RequireDoubleValue(json_text, "n3");
    config.n4 = RequireDoubleValue(json_text, "n4");
    config.n5 = RequireDoubleValue(json_text, "n5");
    config.line_point_count = FindIntValue(json_text, "line_point_count").value_or(200);
    config.reference_c_value = FindDoubleValue(json_text, "reference_c_value").value_or(NaN());
    config.article_reference_y_readoff =
        FindDoubleValue(json_text, "article_reference_y_readoff").value_or(NaN());

    for (const auto& mode_text : RequireStringArrayValues(json_text, "modes")) {
        config.modes.push_back(marcatili::ParseFigure7ModeSpec(mode_text));
    }

    for (const auto& c_text : RequireStringArrayValues(json_text, "c_values")) {
        config.c_lines.push_back(marcatili::ParseFigure7CLineSpec(c_text));
    }

    if (!std::isfinite(config.reference_c_value) && !config.c_lines.empty()) {
        config.reference_c_value = config.c_lines.back().c_value;
    }

    return config;
}

std::string BuildFigure7JsonReport(
    const marcatili::Figure7Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"app\": \"reproduce_fig7\",\n";
    json << "  \"status\": \"" << EscapeJson(result.status) << "\",\n";
    json << "  \"input_file\": " << JsonStringOrNull(input_file) << ",\n";
    json << "  \"output_json_file\": " << JsonStringOrNull(output_json_file) << ",\n";
    json << "  \"output_lines_csv_file\": "
         << JsonStringOrNull(result.config.lines_csv_output_path) << ",\n";
    json << "  \"output_intersections_csv_file\": "
         << JsonStringOrNull(result.config.intersections_csv_output_path) << ",\n";
    json << "  \"case_id\": \"" << EscapeJson(result.config.case_id) << "\",\n";
    json << "  \"article_target\": " << JsonStringOrNull(result.config.article_target) << ",\n";
    json << "  \"geometry\": {\n";
    json << "    \"wavelength\": " << JsonNumber(result.config.wavelength) << ",\n";
    json << "    \"a\": " << JsonNumber(result.config.a) << ",\n";
    json << "    \"b\": " << JsonNumber(result.config.b) << "\n";
    json << "  },\n";
    json << "  \"materials\": {\n";
    json << "    \"n1\": " << JsonNumber(result.config.n1) << ",\n";
    json << "    \"n2\": " << JsonNumber(result.config.n2) << ",\n";
    json << "    \"n3\": " << JsonNumber(result.config.n3) << ",\n";
    json << "    \"n4\": " << JsonNumber(result.config.n4) << ",\n";
    json << "    \"n5\": " << JsonNumber(result.config.n5) << "\n";
    json << "  },\n";
    json << "  \"nomogram\": {\n";
    json << "    \"line_point_count\": " << result.config.line_point_count << ",\n";
    json << "    \"x_numerator\": " << JsonNumber(result.x_numerator) << ",\n";
    json << "    \"y_numerator\": " << JsonNumber(result.y_numerator) << ",\n";
    json << "    \"derived_c\": " << JsonNumber(result.derived_c) << ",\n";
    json << "    \"reference_c_value\": " << JsonNumber(result.config.reference_c_value) << "\n";
    json << "  },\n";
    json << "  \"design_example\": {\n";
    json << "    \"a_over_b\": " << JsonNumber(result.design_example.a_over_b) << ",\n";
    json << "    \"symmetric_material_pairs\": "
         << (result.design_example.symmetric_material_pairs ? "true" : "false") << ",\n";
    json << "    \"delta_from_n35\": "
         << JsonNumberOrNull(result.design_example.delta_from_n35) << ",\n";
    json << "    \"delta_prime_from_n24\": "
         << JsonNumberOrNull(result.design_example.delta_prime_from_n24) << ",\n";
    json << "    \"sqrt_delta_prime_over_delta\": "
         << JsonNumberOrNull(result.design_example.sqrt_delta_prime_over_delta) << "\n";
    json << "  },\n";
    json << "  \"article_reference_check\": ";

    if (result.article_reference_check.available) {
        json << "{\n";
        json << "    \"mode_line_id\": \"" << EscapeJson(result.article_reference_check.mode_line_id)
             << "\",\n";
        json << "    \"reference_c_value\": "
             << JsonNumberOrNull(result.article_reference_check.c_value) << ",\n";
        json << "    \"exact_x\": " << JsonNumberOrNull(result.article_reference_check.exact_x)
             << ",\n";
        json << "    \"exact_y\": " << JsonNumberOrNull(result.article_reference_check.exact_y)
             << ",\n";
        json << "    \"article_y_readoff\": "
             << JsonNumberOrNull(result.article_reference_check.article_y_readoff) << ",\n";
        json << "    \"article_y_absolute_error\": "
             << JsonNumberOrNull(result.article_reference_check.article_y_absolute_error) << ",\n";
        json << "    \"article_y_relative_error\": "
             << JsonNumberOrNull(result.article_reference_check.article_y_relative_error) << ",\n";
        json << "    \"note\": " << JsonStringOrNull(result.article_reference_check.note) << "\n";
        json << "  },\n";
    } else {
        json << "null,\n";
    }

    json << "  \"reference_intersections\": [\n";

    bool first = true;
    for (const auto& intersection : result.intersections) {
        if (!intersection.is_reference_c) {
            continue;
        }

        if (!first) {
            json << ",\n";
        }
        first = false;

        json << "    {\n";
        json << "      \"mode_line_id\": \"" << EscapeJson(intersection.mode_line_id) << "\",\n";
        json << "      \"c_line_id\": \"" << EscapeJson(intersection.c_line_id) << "\",\n";
        json << "      \"mode_family\": \"" << EscapeJson(ToString(intersection.family)) << "\",\n";
        json << "      \"p\": " << intersection.p << ",\n";
        json << "      \"q\": " << intersection.q << ",\n";
        json << "      \"x\": " << JsonNumber(intersection.x) << ",\n";
        json << "      \"y\": " << JsonNumber(intersection.y) << ",\n";
        json << "      \"domain_valid\": " << (intersection.domain_valid ? "true" : "false") << ",\n";
        json << "      \"guided\": " << (intersection.guided ? "true" : "false") << ",\n";
        json << "      \"kz\": " << JsonNumberOrNull(intersection.kz) << ",\n";
        json << "      \"kz_normalized_against_n4\": "
             << JsonNumberOrNull(intersection.kz_normalized_against_n4) << "\n";
        json << "    }";
    }

    json << "\n  ]\n";
    json << "}\n";

    return json.str();
}

std::string BuildFigure7LinesCsvReport(const marcatili::Figure7Result& result) {
    std::ostringstream csv;

    csv << "case_id,line_kind,line_id,mode_family,p,q,c_value,sample_index,x,y\n";

    for (const auto& sample : result.line_samples) {
        csv << result.config.case_id << ","
            << sample.line_kind << ","
            << sample.line_id << ","
            << ToString(sample.family) << ","
            << sample.p << ","
            << sample.q << ","
            << CsvNumber(sample.c_value) << ","
            << sample.sample_index << ","
            << CsvNumber(sample.x) << ","
            << CsvNumber(sample.y) << "\n";
    }

    return csv.str();
}

std::string BuildFigure7IntersectionsCsvReport(const marcatili::Figure7Result& result) {
    std::ostringstream csv;

    csv << "case_id,mode_line_id,c_line_id,mode_family,p,q,c_value,x,y,is_reference_c,"
           "domain_valid,guided,kz,kz_normalized_against_n4\n";

    for (const auto& intersection : result.intersections) {
        csv << result.config.case_id << ","
            << intersection.mode_line_id << ","
            << intersection.c_line_id << ","
            << ToString(intersection.family) << ","
            << intersection.p << ","
            << intersection.q << ","
            << CsvNumber(intersection.c_value) << ","
            << CsvNumber(intersection.x) << ","
            << CsvNumber(intersection.y) << ","
            << (intersection.is_reference_c ? "1" : "0") << ","
            << (intersection.domain_valid ? "1" : "0") << ","
            << (intersection.guided ? "1" : "0") << ","
            << CsvNumber(intersection.kz) << ","
            << CsvNumber(intersection.kz_normalized_against_n4) << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
