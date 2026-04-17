#include "marcatili/io/fig11_io.hpp"

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

std::vector<marcatili::Figure11CurveSpec> ParseCurveSpecs(const std::string& json_text) {
    const auto curve_objects = FindObjectArrayValues(json_text, "curves");
    if (!curve_objects.empty()) {
        std::vector<marcatili::Figure11CurveSpec> curves;
        curves.reserve(curve_objects.size());

        for (const auto& curve_object : curve_objects) {
            marcatili::Figure11CurveSpec curve;
            curve.a_over_A5 = RequireDoubleValue(curve_object, "a_over_A5");
            curve.curve_id = FindStringValue(curve_object, "curve_id").value_or("");
            curve.label = FindStringValue(curve_object, "label").value_or("");
            curves.push_back(curve);
        }

        return curves;
    }

    const auto curve_texts =
        FindStringArrayWithFallback(json_text, "curves", "a_over_A5_values");

    if (curve_texts.empty()) {
        throw std::runtime_error("Missing required key: curves (objects or strings).");
    }

    std::vector<marcatili::Figure11CurveSpec> curves;
    curves.reserve(curve_texts.size());

    for (const auto& curve_text : curve_texts) {
        curves.push_back(marcatili::ParseFigure11CurveSpec(curve_text));
    }

    return curves;
}

std::vector<marcatili::Figure11IndexRatioSpec> ParseIndexRatioSpecs(
    const std::string& json_text
) {
    const auto ratio_objects = FindObjectArrayValues(json_text, "index_ratios");
    if (!ratio_objects.empty()) {
        std::vector<marcatili::Figure11IndexRatioSpec> ratios;
        ratios.reserve(ratio_objects.size());

        for (const auto& ratio_object : ratio_objects) {
            marcatili::Figure11IndexRatioSpec ratio;
            ratio.n1_over_n5 = FindDoubleValue(ratio_object, "n1_over_n5").value_or(0.0);
            ratio.index_ratio_squared =
                FindDoubleValue(ratio_object, "index_ratio_squared").value_or(0.0);
            ratio.ratio_id = FindStringValue(ratio_object, "ratio_id").value_or("");
            ratio.label = FindStringValue(ratio_object, "label").value_or("");
            ratios.push_back(ratio);
        }

        return ratios;
    }

    const auto ratio_texts =
        FindStringArrayWithFallback(json_text, "index_ratios", "n1_over_n5_values");

    if (ratio_texts.empty()) {
        throw std::runtime_error("Missing required key: index_ratios (objects or strings).");
    }

    std::vector<marcatili::Figure11IndexRatioSpec> ratios;
    ratios.reserve(ratio_texts.size());

    for (const auto& ratio_text : ratio_texts) {
        ratios.push_back(marcatili::ParseFigure11IndexRatioSpec(ratio_text));
    }

    return ratios;
}

double RecoverN1OverN5(double index_ratio_squared) {
    if (!(index_ratio_squared > 0.0) || !std::isfinite(index_ratio_squared)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return 1.0 / std::sqrt(index_ratio_squared);
}

}  // namespace

marcatili::Figure11Config ParseFigure11Config(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::Figure11Config config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");
    config.ocr_note = FindStringValue(json_text, "ocr_note").value_or("");

    config.csv_output_path =
        FindStringValue(json_text, "csv_file")
            .value_or(BuildDefaultCsvPath(cli_output_json));

    config.c_over_a_min =
        RequireDoubleWithFallback(json_text, "sweep.c_over_a_min", "c_over_a_min");
    config.c_over_a_max =
        RequireDoubleWithFallback(json_text, "sweep.c_over_a_max", "c_over_a_max");
    config.point_count =
        RequireIntWithFallback(json_text, "sweep.point_count", "point_count");

    const auto solver_model_texts = FindStringArrayValues(json_text, "solver_models");
    if (solver_model_texts.empty()) {
        config.solver_models.push_back(marcatili::SingleGuideSolverModel::kExact);
    } else {
        for (const auto& solver_model_text : solver_model_texts) {
            config.solver_models.push_back(
                marcatili::ParseSingleGuideSolverModel(solver_model_text)
            );
        }
    }

    config.curves = ParseCurveSpecs(json_text);
    config.index_ratios = ParseIndexRatioSpecs(json_text);

    return config;
}

std::string BuildFigure11JsonReport(
    const marcatili::Figure11Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;
    json << "{\n";

    AppendJsonField(json, "app", "\"reproduce_fig11\"");
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
            "Current Fig. 11 reproduction uses Eq. (34) together with the transverse root "
            "from Eq. (20), as stated explicitly in Section IV. The repository baseline "
            "tracks the two line-style families mentioned in the caption: n1/n5 = 1.5 and "
            "n1/n5 = 1.1. Eq. (22) can be enabled later for a closed-form comparison, but "
            "the default article-style case keeps only the exact curves."
        )
    );

    json << "  \"sweep\": {\n";
    AppendJsonField(json, "c_over_a_min", JsonNumber(result.config.c_over_a_min), true, 4);
    AppendJsonField(json, "c_over_a_max", JsonNumber(result.config.c_over_a_max), true, 4);
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

    json << "  \"curve_summaries\": [\n";
    for (std::size_t index = 0; index < result.curves.size(); ++index) {
        const auto& curve = result.curves[index];

        json << "    {\n";
        AppendJsonField(json, "ratio_id", "\"" + EscapeJson(curve.index_ratio.ratio_id) + "\"", true, 6);
        AppendJsonField(json, "ratio_label", "\"" + EscapeJson(curve.index_ratio.label) + "\"", true, 6);
        AppendJsonField(json, "n1_over_n5", JsonNumber(curve.index_ratio.n1_over_n5), true, 6);
        AppendJsonField(
            json,
            "index_ratio_squared",
            JsonNumber(curve.index_ratio.index_ratio_squared),
            true,
            6
        );
        AppendJsonField(json, "curve_id", "\"" + EscapeJson(curve.curve.curve_id) + "\"", true, 6);
        AppendJsonField(json, "curve_label", "\"" + EscapeJson(curve.curve.label) + "\"", true, 6);
        AppendJsonField(json, "a_over_A5", JsonNumber(curve.curve.a_over_A5), true, 6);
        AppendJsonField(
            json,
            "solver_model",
            "\"" + EscapeJson(ToString(curve.solver_model)) + "\"",
            true,
            6
        );
        AppendJsonField(
            json,
            "kx_A5_over_pi",
            JsonNumberOrNull(curve.kx_A5_over_pi),
            true,
            6
        );
        AppendJsonField(json, "total_points", std::to_string(curve.total_points), true, 6);
        AppendJsonField(json, "valid_points", std::to_string(curve.valid_points), false, 6);
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

std::string BuildFigure11CsvReport(const marcatili::Figure11Result& result) {
    std::ostringstream csv;

    csv << "case_id,ratio_id,ratio_label,curve_id,curve_label,solver_model,"
           "transverse_equation,p,n1_over_n5,index_ratio_squared,a_over_A5,sample_index,"
           "c_over_a,c_over_A5,kx_A5_over_pi,sqrt_one_minus_kx_A5_over_pi_squared,"
           "normalized_coupling,domain_valid,status\n";

    for (const auto& sample : result.samples) {
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(sample.ratio_id) << ","
            << EscapeCsv(sample.ratio_label) << ","
            << EscapeCsv(sample.curve_id) << ","
            << EscapeCsv(sample.curve_label) << ","
            << EscapeCsv(ToString(sample.point.config.solver_model)) << ","
            << EscapeCsv(ToString(sample.point.config.transverse_equation)) << ","
            << sample.point.config.p << ","
            << CsvNumber(RecoverN1OverN5(sample.point.config.index_ratio_squared)) << ","
            << CsvNumber(sample.point.config.index_ratio_squared) << ","
            << CsvNumber(sample.point.config.a_over_A5) << ","
            << sample.sample_index << ","
            << CsvNumber(sample.c_over_a) << ","
            << CsvNumber(sample.point.c_over_A5) << ","
            << CsvNumber(sample.point.kx_A5_over_pi) << ","
            << CsvNumber(sample.point.sqrt_one_minus_kx_A5_over_pi_squared) << ","
            << CsvNumber(sample.point.normalized_coupling) << ","
            << (sample.point.domain_valid ? "1" : "0") << ","
            << EscapeCsv(sample.point.status) << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
