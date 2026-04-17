#include "marcatili/io/fig11_io.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

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

marcatili::Figure11Config ParseFigure11Config(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::Figure11Config config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");
    config.ocr_note = FindStringValue(json_text, "ocr_note").value_or("");
    config.csv_output_path =
        FindStringValue(json_text, "csv_file").value_or(DefaultCsvPath(cli_output_json));
    config.c_over_a_min = RequireDoubleValue(json_text, "c_over_a_min");
    config.c_over_a_max = RequireDoubleValue(json_text, "c_over_a_max");
    config.point_count = RequireIntValue(json_text, "point_count");

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

    for (const auto& curve_text : RequireStringArrayValues(json_text, "a_over_A5_values")) {
        config.curves.push_back(marcatili::ParseFigure11CurveSpec(curve_text));
    }

    for (const auto& ratio_text : RequireStringArrayValues(json_text, "n1_over_n5_values")) {
        config.index_ratios.push_back(marcatili::ParseFigure11IndexRatioSpec(ratio_text));
    }

    return config;
}

std::string BuildFigure11JsonReport(
    const marcatili::Figure11Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"app\": \"reproduce_fig11\",\n";
    json << "  \"status\": \"" << EscapeJson(result.status) << "\",\n";
    json << "  \"input_file\": " << JsonStringOrNull(input_file) << ",\n";
    json << "  \"output_json_file\": " << JsonStringOrNull(output_json_file) << ",\n";
    json << "  \"output_csv_file\": " << JsonStringOrNull(result.config.csv_output_path) << ",\n";
    json << "  \"case_id\": \"" << EscapeJson(result.config.case_id) << "\",\n";
    json << "  \"article_target\": " << JsonStringOrNull(result.config.article_target) << ",\n";
    json << "  \"ocr_note\": " << JsonStringOrNull(result.config.ocr_note) << ",\n";
    json << "  \"model_note\": "
         << JsonStringOrNull(
                "Current Fig. 11 reproduction uses Eq. (34) together with the transverse root "
                "from Eq. (20), as stated explicitly in Section IV. The repository baseline "
                "tracks the two line-style families mentioned in the caption: n1/n5 = 1.5 and "
                "n1/n5 = 1.1. Eq. (22) can be enabled later for a closed-form comparison, but "
                "the default article-style case keeps only the exact curves."
            ) << ",\n";
    json << "  \"sweep\": {\n";
    json << "    \"c_over_a_min\": " << JsonNumber(result.config.c_over_a_min) << ",\n";
    json << "    \"c_over_a_max\": " << JsonNumber(result.config.c_over_a_max) << ",\n";
    json << "    \"point_count\": " << result.config.point_count << "\n";
    json << "  },\n";
    json << "  \"curve_summaries\": [\n";

    for (std::size_t index = 0; index < result.curves.size(); ++index) {
        const auto& curve = result.curves[index];
        json << "    {\n";
        json << "      \"ratio_id\": \"" << EscapeJson(curve.index_ratio.ratio_id) << "\",\n";
        json << "      \"ratio_label\": \"" << EscapeJson(curve.index_ratio.label) << "\",\n";
        json << "      \"n1_over_n5\": " << JsonNumber(curve.index_ratio.n1_over_n5) << ",\n";
        json << "      \"index_ratio_squared\": "
             << JsonNumber(curve.index_ratio.index_ratio_squared) << ",\n";
        json << "      \"curve_id\": \"" << EscapeJson(curve.curve.curve_id) << "\",\n";
        json << "      \"curve_label\": \"" << EscapeJson(curve.curve.label) << "\",\n";
        json << "      \"a_over_A5\": " << JsonNumber(curve.curve.a_over_A5) << ",\n";
        json << "      \"solver_model\": \"" << EscapeJson(ToString(curve.solver_model)) << "\",\n";
        json << "      \"kx_A5_over_pi\": " << JsonNumberOrNull(curve.kx_A5_over_pi) << ",\n";
        json << "      \"total_points\": " << curve.total_points << ",\n";
        json << "      \"valid_points\": " << curve.valid_points << "\n";
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
        csv << result.config.case_id << ","
            << sample.ratio_id << ","
            << sample.ratio_label << ","
            << sample.curve_id << ","
            << sample.curve_label << ","
            << ToString(sample.point.config.solver_model) << ","
            << ToString(sample.point.config.transverse_equation) << ","
            << sample.point.config.p << ","
            << CsvNumber(1.0 / std::sqrt(sample.point.config.index_ratio_squared)) << ","
            << CsvNumber(sample.point.config.index_ratio_squared) << ","
            << CsvNumber(sample.point.a_over_A5) << ","
            << sample.sample_index << ","
            << CsvNumber(sample.c_over_a) << ","
            << CsvNumber(sample.point.c_over_A5) << ","
            << CsvNumber(sample.point.kx_A5_over_pi) << ","
            << CsvNumber(sample.point.sqrt_one_minus_kx_A5_over_pi_squared) << ","
            << CsvNumber(sample.point.normalized_coupling) << ","
            << (sample.point.domain_valid ? "1" : "0") << ","
            << sample.point.status << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
