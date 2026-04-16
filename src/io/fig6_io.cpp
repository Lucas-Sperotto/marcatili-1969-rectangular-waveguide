#include "marcatili/io/fig6_io.hpp"

#include <cmath>
#include <iomanip>
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
    if (!std::isfinite(value) || value <= 0.0) {
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

marcatili::Figure6MaterialVariant ParseFigure6MaterialVariant(const std::string& text) {
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
    variant.variant_id = variant_id;
    variant.n2 = std::stod(n2_text);
    variant.n3 = std::stod(n3_text);
    variant.n4 = std::stod(n4_text);
    variant.n5 = std::stod(n5_text);
    return variant;
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
        FindStringValue(json_text, "csv_file").value_or(DefaultCsvPath(cli_output_json));
    config.geometry_model = marcatili::ParseFigure6GeometryModel(
        FindStringValue(json_text, "geometry_model").value_or("rectangular")
    );
    config.wavelength = RequireDoubleValue(json_text, "wavelength");
    config.a_over_b =
        config.geometry_model == marcatili::Figure6GeometryModel::kRectangular
            ? RequireDoubleValue(json_text, "a_over_b")
            : FindDoubleValue(json_text, "a_over_b").value_or(0.0);
    config.n1 = RequireDoubleValue(json_text, "n1");
    config.n2 = RequireDoubleValue(json_text, "n2");
    config.n3 = RequireDoubleValue(json_text, "n3");
    config.n4 = RequireDoubleValue(json_text, "n4");
    config.n5 = RequireDoubleValue(json_text, "n5");
    config.b_over_A4_min = RequireDoubleValue(json_text, "b_over_A4_min");
    config.b_over_A4_max = RequireDoubleValue(json_text, "b_over_A4_max");
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
        config.modes.push_back(marcatili::ParseFigure6ModeSpec(mode_text));
    }

    for (const auto& variant_text : FindStringArrayValues(json_text, "material_variants")) {
        config.material_variants.push_back(ParseFigure6MaterialVariant(variant_text));
    }

    return config;
}

std::string BuildFigure6JsonReport(
    const marcatili::Figure6Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"app\": \"reproduce_fig6\",\n";
    json << "  \"status\": \"" << EscapeJson(result.status) << "\",\n";
    json << "  \"model\": \"closed_form_approximation\",\n";
    json << "  \"input_file\": " << JsonStringOrNull(input_file) << ",\n";
    json << "  \"output_json_file\": " << JsonStringOrNull(output_json_file) << ",\n";
    json << "  \"output_csv_file\": " << JsonStringOrNull(result.config.csv_output_path) << ",\n";
    json << "  \"case_id\": \"" << EscapeJson(result.config.case_id) << "\",\n";
    json << "  \"article_target\": " << JsonStringOrNull(result.config.article_target) << ",\n";
    json << "  \"panel_id\": \"" << EscapeJson(result.config.panel_id) << "\",\n";
    json << "  \"sweep\": {\n";
    json << "    \"geometry_model\": \""
         << EscapeJson(ToString(result.config.geometry_model)) << "\",\n";
    json << "    \"wavelength\": " << JsonNumber(result.config.wavelength) << ",\n";
    json << "    \"a_over_b\": " << JsonNumberOrNull(result.config.a_over_b) << ",\n";
    json << "    \"b_over_A4_min\": " << JsonNumber(result.config.b_over_A4_min) << ",\n";
    json << "    \"b_over_A4_max\": " << JsonNumber(result.config.b_over_A4_max) << ",\n";
    json << "    \"point_count\": " << result.config.point_count << "\n";
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
    json << "    \"n1\": " << JsonNumber(result.config.n1) << ",\n";
    json << "    \"n2\": " << JsonNumber(result.config.n2) << ",\n";
    json << "    \"n3\": " << JsonNumber(result.config.n3) << ",\n";
    json << "    \"n4\": " << JsonNumber(result.config.n4) << ",\n";
    json << "    \"n5\": " << JsonNumber(result.config.n5) << "\n";
    json << "  },\n";
    json << "  \"material_variants\": [\n";
    for (std::size_t index = 0; index < result.config.material_variants.size(); ++index) {
        const auto& variant = result.config.material_variants[index];
        json << "    {\n";
        json << "      \"variant_id\": \"" << EscapeJson(variant.variant_id) << "\",\n";
        json << "      \"n2\": " << JsonNumber(variant.n2) << ",\n";
        json << "      \"n3\": " << JsonNumber(variant.n3) << ",\n";
        json << "      \"n4\": " << JsonNumber(variant.n4) << ",\n";
        json << "      \"n5\": " << JsonNumber(variant.n5) << "\n";
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
        json << "      \"variant_id\": \"" << EscapeJson(curve.variant_id) << "\",\n";
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

std::string BuildFigure6CsvReport(const marcatili::Figure6Result& result) {
    std::ostringstream csv;

    csv << "case_id,panel_id,geometry_model,variant_id,curve_id,solver_model,mode_family,p,q,sample_index,"
           "b_over_A4,a,b,guided,domain_valid,"
           "kx,ky,kz,kz_normalized_against_n4,xi3,xi5,eta2,eta4\n";

    for (const auto& sample : result.samples) {
        csv << result.config.case_id << ","
            << sample.panel_id << ","
            << ToString(result.config.geometry_model) << ","
            << sample.variant_id << ","
            << sample.curve_id << ","
            << ToString(sample.point.config.solver_model) << ","
            << ToString(sample.point.config.family) << ","
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
