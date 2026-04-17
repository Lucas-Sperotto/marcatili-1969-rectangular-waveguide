#include "marcatili/io/fig6_io.hpp"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
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

std::string JsonPositiveNumberOrNull(double value) {
    if (!std::isfinite(value) || value <= 0.0) {
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

std::string TrimWhitespaceCopy(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() &&
           std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }

    return text.substr(begin, end - begin);
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

marcatili::Figure6MaterialVariant ParseFigure6MaterialVariantLegacy(
    const std::string& text
) {
    std::stringstream parser(text);
    std::string variant_id;
    std::string n2_text;
    std::string n3_text;
    std::string n4_text;
    std::string n5_text;

    if (!std::getline(parser, variant_id, ':') ||
        !std::getline(parser, n2_text, ':') ||
        !std::getline(parser, n3_text, ':') ||
        !std::getline(parser, n4_text, ':') ||
        !std::getline(parser, n5_text, ':')) {
        throw std::runtime_error(
            "Invalid material_variants entry: " + text +
            ". Use variant_id:n2:n3:n4:n5."
        );
    }

    marcatili::Figure6MaterialVariant variant;

    try {
        variant.variant_id = variant_id;
        variant.n2 = std::stod(n2_text);
        variant.n3 = std::stod(n3_text);
        variant.n4 = std::stod(n4_text);
        variant.n5 = std::stod(n5_text);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Invalid numeric values in material_variants entry: " + text
        );
    }

    return variant;
}

marcatili::Figure6MaterialVariant ParseFigure6MaterialVariantObject(
    const std::string& object_json
) {
    marcatili::Figure6MaterialVariant variant;
    variant.variant_id = RequireStringValue(object_json, "variant_id");
    variant.n2 = RequireDoubleValue(object_json, "n2");
    variant.n3 = RequireDoubleValue(object_json, "n3");
    variant.n4 = RequireDoubleValue(object_json, "n4");
    variant.n5 = RequireDoubleValue(object_json, "n5");
    return variant;
}

std::vector<marcatili::Figure6MaterialVariant> ParseMaterialVariants(
    const std::string& json_text
) {
    std::vector<marcatili::Figure6MaterialVariant> variants;

    const auto object_variants = FindObjectArrayValues(json_text, "material_variants");
    if (!object_variants.empty()) {
        for (const auto& variant_object : object_variants) {
            variants.push_back(ParseFigure6MaterialVariantObject(variant_object));
        }

        return variants;
    }

    const auto string_variants = FindStringArrayValues(json_text, "material_variants");
    if (!string_variants.empty()) {
        for (const auto& variant_text : string_variants) {
            variants.push_back(ParseFigure6MaterialVariantLegacy(variant_text));
        }

        return variants;
    }

    const auto raw_variants = FindRawJsonValue(json_text, "material_variants");
    if (raw_variants.has_value()) {
        const std::string trimmed = TrimWhitespaceCopy(*raw_variants);

        // Permite explicitamente array vazio.
        if (trimmed == "[]") {
            return variants;
        }

        throw std::runtime_error(
            "Invalid material_variants format. Use either "
            "an array of objects "
            "([{variant_id,n2,n3,n4,n5}, ...]) "
            "or the legacy compact string format."
        );
    }

    return variants;
}

}  // namespace

marcatili::Figure6Config ParseFigure6Config(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::Figure6Config config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");
    config.panel_id = RequireStringValue(json_text, "panel_id");

    config.csv_output_path =
        FindStringValue(json_text, "csv_file")
            .value_or(BuildDefaultCsvPath(cli_output_json));

    config.geometry_model = marcatili::ParseFigure6GeometryModel(
        FindStringValue(json_text, "geometry_model").value_or("rectangular")
    );

    config.wavelength =
        RequireDoubleWithFallback(json_text, "geometry.wavelength", "wavelength");

    config.a_over_b =
        (config.geometry_model == marcatili::Figure6GeometryModel::kRectangular)
            ? RequireDoubleWithFallback(json_text, "geometry.a_over_b", "a_over_b")
            : FindDoubleWithFallback(json_text, "geometry.a_over_b", "a_over_b").value_or(0.0);

    config.n1 = RequireDoubleWithFallback(json_text, "materials.n1", "n1");
    config.n2 = RequireDoubleWithFallback(json_text, "materials.n2", "n2");
    config.n3 = RequireDoubleWithFallback(json_text, "materials.n3", "n3");
    config.n4 = RequireDoubleWithFallback(json_text, "materials.n4", "n4");
    config.n5 = RequireDoubleWithFallback(json_text, "materials.n5", "n5");

    config.b_over_A4_min =
        RequireDoubleWithFallback(json_text, "sweep.b_over_A4_min", "b_over_A4_min");
    config.b_over_A4_max =
        RequireDoubleWithFallback(json_text, "sweep.b_over_A4_max", "b_over_A4_max");
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
        config.modes.push_back(marcatili::ParseFigure6ModeSpec(mode_text));
    }

    config.material_variants = ParseMaterialVariants(json_text);

    return config;
}

std::string BuildFigure6JsonReport(
    const marcatili::Figure6Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;
    json << "{\n";

    AppendJsonField(json, "app", "\"reproduce_fig6\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(json, "app_model", "\"figure6_sweep\"");
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(json, "output_csv_file", JsonStringOrNull(result.config.csv_output_path));
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));
    AppendJsonField(json, "panel_id", "\"" + EscapeJson(result.config.panel_id) + "\"");

    json << "  \"sweep\": {\n";
    AppendJsonField(
        json,
        "geometry_model",
        "\"" + EscapeJson(ToString(result.config.geometry_model)) + "\"",
        true,
        4
    );
    AppendJsonField(json, "wavelength", JsonNumber(result.config.wavelength), true, 4);
    AppendJsonField(json, "a_over_b", JsonPositiveNumberOrNull(result.config.a_over_b), true, 4);
    AppendJsonField(json, "b_over_A4_min", JsonNumber(result.config.b_over_A4_min), true, 4);
    AppendJsonField(json, "b_over_A4_max", JsonNumber(result.config.b_over_A4_max), true, 4);
    AppendJsonField(json, "point_count", std::to_string(result.config.point_count), false, 4);
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

    json << "  \"materials\": {\n";
    AppendJsonField(json, "n1", JsonNumber(result.config.n1), true, 4);
    AppendJsonField(json, "n2", JsonNumber(result.config.n2), true, 4);
    AppendJsonField(json, "n3", JsonNumber(result.config.n3), true, 4);
    AppendJsonField(json, "n4", JsonNumber(result.config.n4), true, 4);
    AppendJsonField(json, "n5", JsonNumber(result.config.n5), false, 4);
    json << "  },\n";

    json << "  \"material_variants\": [\n";
    for (std::size_t index = 0; index < result.config.material_variants.size(); ++index) {
        const auto& variant = result.config.material_variants[index];

        json << "    {\n";
        AppendJsonField(json, "variant_id", "\"" + EscapeJson(variant.variant_id) + "\"", true, 6);
        AppendJsonField(json, "n2", JsonNumber(variant.n2), true, 6);
        AppendJsonField(json, "n3", JsonNumber(variant.n3), true, 6);
        AppendJsonField(json, "n4", JsonNumber(variant.n4), true, 6);
        AppendJsonField(json, "n5", JsonNumber(variant.n5), false, 6);
        json << "    }";

        if (index + 1 != result.config.material_variants.size()) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";

    json << "  \"curve_summaries\": [\n";
    for (std::size_t index = 0; index < result.curves.size(); ++index) {
        const auto& curve = result.curves[index];

        json << "    {\n";
        AppendJsonField(json, "variant_id", "\"" + EscapeJson(curve.variant_id) + "\"", true, 6);
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

std::string BuildFigure6CsvReport(const marcatili::Figure6Result& result) {
    std::ostringstream csv;

    csv << "case_id,panel_id,geometry_model,variant_id,curve_id,solver_model,mode_family,p,q,sample_index,"
           "b_over_A4,a,b,guided,domain_valid,"
           "kx,ky,kz,kz_normalized_against_n4,xi3,xi5,eta2,eta4\n";

    for (const auto& sample : result.samples) {
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(sample.panel_id) << ","
            << EscapeCsv(ToString(result.config.geometry_model)) << ","
            << EscapeCsv(sample.variant_id) << ","
            << EscapeCsv(sample.curve_id) << ","
            << EscapeCsv(ToString(sample.point.config.solver_model)) << ","
            << EscapeCsv(ToString(sample.point.config.family)) << ","
            << sample.point.config.p << ","
            << sample.point.config.q << ","
            << sample.sample_index << ","
            << CsvNumber(sample.b_over_A4) << ","
            << CsvNumber(sample.point.config.a) << ","
            << CsvNumber(sample.point.config.b) << ","
            << (sample.point.guided ? "1" : "0") << ","
            << (sample.point.domain_valid ? "1" : "0") << ","
            << CsvNumber(sample.point.kx) << ","
            << CsvNumber(sample.point.ky) << ","
            << CsvNumber(sample.point.kz) << ","
            << CsvNumber(sample.point.kz_normalized_against_n4) << ","
            << CsvNumber(sample.point.xi3) << ","
            << CsvNumber(sample.point.xi5) << ","
            << CsvNumber(sample.point.eta2) << ","
            << CsvNumber(sample.point.eta4) << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
