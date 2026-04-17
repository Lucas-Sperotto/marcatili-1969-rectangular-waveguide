#include "marcatili/io/fig7_io.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
    if (!std::isfinite(value)) {
        return "nan";
    }

    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
}

std::string BuildDefaultLinesCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".lines.csv");
}

std::string BuildDefaultIntersectionsCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".intersections.csv");
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

std::optional<std::string> FindStringWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto dotted_value = FindStringValue(json_text, dotted_key);
    if (dotted_value.has_value()) {
        return dotted_value;
    }

    return FindStringValue(json_text, flat_key);
}

std::vector<std::string> FindStringArrayWithFallback(
    const std::string& json_text,
    const std::string& dotted_key,
    const std::string& flat_key
) {
    const auto dotted_values = FindStringArrayValues(json_text, dotted_key);
    if (!dotted_values.empty()) {
        return dotted_values;
    }

    return FindStringArrayValues(json_text, flat_key);
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

std::string ResolveArticleReferenceModeLineId(const std::string& json_text) {
    if (const auto line_id =
            FindStringWithFallback(json_text, "article_reference.mode_line_id", "article_reference_mode_line_id")) {
        return *line_id;
    }

    if (const auto mode_text =
            FindStringWithFallback(json_text, "article_reference.mode", "article_reference_mode")) {
        return marcatili::ParseFigure7ModeSpec(*mode_text).line_id;
    }

    return "";
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
        FindStringValue(json_text, "csv_lines_file")
            .value_or(BuildDefaultLinesCsvPath(cli_output_json));

    config.intersections_csv_output_path =
        FindStringValue(json_text, "csv_intersections_file")
            .value_or(BuildDefaultIntersectionsCsvPath(cli_output_json));

    config.article_reference_mode_line_id = ResolveArticleReferenceModeLineId(json_text);

    config.article_reference_note =
        FindStringWithFallback(json_text, "article_reference.note", "article_reference_note")
            .value_or("");

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

    config.line_point_count =
        FindIntWithFallback(json_text, "nomogram.line_point_count", "line_point_count")
            .value_or(200);

    config.reference_c_value =
        FindDoubleWithFallback(json_text, "nomogram.reference_c_value", "reference_c_value")
            .value_or(NaN());

    config.article_reference_y_readoff =
        FindDoubleWithFallback(
            json_text,
            "article_reference.y_readoff",
            "article_reference_y_readoff"
        ).value_or(NaN());

    for (const auto& mode_text : RequireStringArrayValues(json_text, "modes")) {
        config.modes.push_back(marcatili::ParseFigure7ModeSpec(mode_text));
    }

    const auto c_texts = FindStringArrayWithFallback(json_text, "c_lines", "c_values");
    if (c_texts.empty()) {
        throw std::runtime_error("Missing required string-array key: c_lines (or c_values)");
    }

    for (const auto& c_text : c_texts) {
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

    AppendJsonField(json, "app", "\"reproduce_fig7\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(
        json,
        "output_lines_csv_file",
        JsonStringOrNull(result.config.lines_csv_output_path)
    );
    AppendJsonField(
        json,
        "output_intersections_csv_file",
        JsonStringOrNull(result.config.intersections_csv_output_path)
    );
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));

    json << "  \"geometry\": {\n";
    AppendJsonField(json, "wavelength", JsonNumber(result.config.wavelength), true, 4);
    AppendJsonField(json, "a", JsonNumber(result.config.a), true, 4);
    AppendJsonField(json, "b", JsonNumber(result.config.b), false, 4);
    json << "  },\n";

    json << "  \"materials\": {\n";
    AppendJsonField(json, "n1", JsonNumber(result.config.n1), true, 4);
    AppendJsonField(json, "n2", JsonNumber(result.config.n2), true, 4);
    AppendJsonField(json, "n3", JsonNumber(result.config.n3), true, 4);
    AppendJsonField(json, "n4", JsonNumber(result.config.n4), true, 4);
    AppendJsonField(json, "n5", JsonNumber(result.config.n5), false, 4);
    json << "  },\n";

    json << "  \"nomogram\": {\n";
    AppendJsonField(json, "line_point_count", std::to_string(result.config.line_point_count), true, 4);
    AppendJsonField(json, "x_numerator", JsonNumber(result.x_numerator), true, 4);
    AppendJsonField(json, "y_numerator", JsonNumber(result.y_numerator), true, 4);
    AppendJsonField(json, "derived_c", JsonNumber(result.derived_c), true, 4);
    AppendJsonField(
        json,
        "reference_c_value",
        JsonNumberOrNull(result.config.reference_c_value),
        true,
        4
    );
    AppendJsonField(
        json,
        "reference_c_absolute_error",
        JsonNumberOrNull(result.reference_c_absolute_error),
        true,
        4
    );
    AppendJsonField(
        json,
        "reference_c_relative_error",
        JsonNumberOrNull(result.reference_c_relative_error),
        false,
        4
    );
    json << "  },\n";

    json << "  \"design_example\": {\n";
    AppendJsonField(json, "a_over_b", JsonNumber(result.design_example.a_over_b), true, 4);
    AppendJsonField(
        json,
        "symmetric_material_pairs",
        result.design_example.symmetric_material_pairs ? "true" : "false",
        true,
        4
    );
    AppendJsonField(
        json,
        "delta_from_n35",
        JsonNumberOrNull(result.design_example.delta_from_n35),
        true,
        4
    );
    AppendJsonField(
        json,
        "delta_prime_from_n24",
        JsonNumberOrNull(result.design_example.delta_prime_from_n24),
        true,
        4
    );
    AppendJsonField(
        json,
        "sqrt_delta_prime_over_delta",
        JsonNumberOrNull(result.design_example.sqrt_delta_prime_over_delta),
        false,
        4
    );
    json << "  },\n";

    json << "  \"article_reference_check\": ";
    if (result.article_reference_check.available) {
        json << "{\n";
        AppendJsonField(
            json,
            "mode_line_id",
            "\"" + EscapeJson(result.article_reference_check.mode_line_id) + "\"",
            true,
            4
        );
        AppendJsonField(
            json,
            "reference_c_value",
            JsonNumberOrNull(result.article_reference_check.c_value),
            true,
            4
        );
        AppendJsonField(
            json,
            "exact_x",
            JsonNumberOrNull(result.article_reference_check.exact_x),
            true,
            4
        );
        AppendJsonField(
            json,
            "exact_y",
            JsonNumberOrNull(result.article_reference_check.exact_y),
            true,
            4
        );
        AppendJsonField(
            json,
            "article_y_readoff",
            JsonNumberOrNull(result.article_reference_check.article_y_readoff),
            true,
            4
        );
        AppendJsonField(
            json,
            "article_y_absolute_error",
            JsonNumberOrNull(result.article_reference_check.article_y_absolute_error),
            true,
            4
        );
        AppendJsonField(
            json,
            "article_y_relative_error",
            JsonNumberOrNull(result.article_reference_check.article_y_relative_error),
            true,
            4
        );
        AppendJsonField(
            json,
            "note",
            JsonStringOrNull(result.article_reference_check.note),
            false,
            4
        );
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
        AppendJsonField(
            json,
            "mode_line_id",
            "\"" + EscapeJson(intersection.mode_line_id) + "\"",
            true,
            6
        );
        AppendJsonField(
            json,
            "c_line_id",
            "\"" + EscapeJson(intersection.c_line_id) + "\"",
            true,
            6
        );
        AppendJsonField(
            json,
            "mode_family",
            "\"" + EscapeJson(ToString(intersection.family)) + "\"",
            true,
            6
        );
        AppendJsonField(json, "p", std::to_string(intersection.p), true, 6);
        AppendJsonField(json, "q", std::to_string(intersection.q), true, 6);
        AppendJsonField(json, "x", JsonNumber(intersection.x), true, 6);
        AppendJsonField(json, "y", JsonNumber(intersection.y), true, 6);
        AppendJsonField(
            json,
            "domain_valid",
            intersection.domain_valid ? "true" : "false",
            true,
            6
        );
        AppendJsonField(
            json,
            "guided",
            intersection.guided ? "true" : "false",
            true,
            6
        );
        AppendJsonField(json, "kz", JsonNumberOrNull(intersection.kz), true, 6);
        AppendJsonField(
            json,
            "kz_normalized_against_n4",
            JsonNumberOrNull(intersection.kz_normalized_against_n4),
            false,
            6
        );
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
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(sample.line_kind) << ","
            << EscapeCsv(sample.line_id) << ","
            << EscapeCsv(ToString(sample.family)) << ","
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
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(intersection.mode_line_id) << ","
            << EscapeCsv(intersection.c_line_id) << ","
            << EscapeCsv(ToString(intersection.family)) << ","
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
