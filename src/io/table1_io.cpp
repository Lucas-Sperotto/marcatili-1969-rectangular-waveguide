#include "marcatili/io/table1_io.hpp"

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

std::string DefaultSummaryCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".summary.csv");
}

std::string DefaultDetailsCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".details.csv");
}

marcatili::Table1RowSpec ParseTable1RowSpec(const std::string& text) {
    std::stringstream parser(text);
    std::string row_id;
    std::string article_panel_id;
    std::string a_over_b_text;
    std::string n2_text;
    std::string n3_text;
    std::string n4_text;
    std::string n5_text;
    std::string article_dimension_text;

    if (!std::getline(parser, row_id, ':') ||
        !std::getline(parser, article_panel_id, ':') ||
        !std::getline(parser, a_over_b_text, ':') ||
        !std::getline(parser, n2_text, ':') ||
        !std::getline(parser, n3_text, ':') ||
        !std::getline(parser, n4_text, ':') ||
        !std::getline(parser, n5_text, ':') ||
        !std::getline(parser, article_dimension_text, ':')) {
        throw std::runtime_error(
            "Invalid rows entry: " + text +
            ". Use row_id:article_panel_id:a_over_b:n2:n3:n4:n5:article_dimension_normalized."
        );
    }

    marcatili::Table1RowSpec row;
    row.row_id = row_id;
    row.article_panel_id = article_panel_id;
    row.a_over_b = std::stod(a_over_b_text);
    row.n2 = std::stod(n2_text);
    row.n3 = std::stod(n3_text);
    row.n4 = std::stod(n4_text);
    row.n5 = std::stod(n5_text);
    row.article_dimension_normalized = std::stod(article_dimension_text);
    return row;
}

}  // namespace

marcatili::Table1Config ParseTable1Config(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::Table1Config config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");
    config.summary_csv_output_path =
        FindStringValue(json_text, "csv_summary_file")
            .value_or(DefaultSummaryCsvPath(cli_output_json));
    config.details_csv_output_path =
        FindStringValue(json_text, "csv_details_file")
            .value_or(DefaultDetailsCsvPath(cli_output_json));
    config.wavelength = RequireDoubleValue(json_text, "wavelength");
    config.n1 = RequireDoubleValue(json_text, "n1");
    config.search.max_p = FindIntValue(json_text, "search_max_p").value_or(4);
    config.search.max_q = FindIntValue(json_text, "search_max_q").value_or(4);
    config.search.b_normalized_min =
        FindDoubleValue(json_text, "search_b_normalized_min").value_or(0.01);
    config.search.b_normalized_max =
        FindDoubleValue(json_text, "search_b_normalized_max").value_or(60.0);
    config.search.cutoff_tolerance =
        FindDoubleValue(json_text, "search_cutoff_tolerance").value_or(1e-6);

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

    for (const auto& row_text : RequireStringArrayValues(json_text, "rows")) {
        config.rows.push_back(ParseTable1RowSpec(row_text));
    }

    return config;
}

std::string BuildTable1JsonReport(
    const marcatili::Table1Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"app\": \"reproduce_table1\",\n";
    json << "  \"status\": \"" << EscapeJson(result.status) << "\",\n";
    json << "  \"input_file\": " << JsonStringOrNull(input_file) << ",\n";
    json << "  \"output_json_file\": " << JsonStringOrNull(output_json_file) << ",\n";
    json << "  \"output_summary_csv_file\": "
         << JsonStringOrNull(result.config.summary_csv_output_path) << ",\n";
    json << "  \"output_details_csv_file\": "
         << JsonStringOrNull(result.config.details_csv_output_path) << ",\n";
    json << "  \"case_id\": \"" << EscapeJson(result.config.case_id) << "\",\n";
    json << "  \"article_target\": " << JsonStringOrNull(result.config.article_target) << ",\n";
    json << "  \"reference_normalization\": \"table entries multiplied by lambda / n1 and interpreted here as dimension a\",\n";
    json << "  \"search\": {\n";
    json << "    \"max_p\": " << result.config.search.max_p << ",\n";
    json << "    \"max_q\": " << result.config.search.max_q << ",\n";
    json << "    \"b_normalized_min\": " << JsonNumber(result.config.search.b_normalized_min) << ",\n";
    json << "    \"b_normalized_max\": " << JsonNumber(result.config.search.b_normalized_max) << ",\n";
    json << "    \"cutoff_tolerance\": " << JsonNumber(result.config.search.cutoff_tolerance) << "\n";
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
    json << "  \"row_summaries\": [\n";

    for (std::size_t index = 0; index < result.row_summaries.size(); ++index) {
        const auto& row = result.row_summaries[index];
        json << "    {\n";
        json << "      \"row_id\": \"" << EscapeJson(row.row_id) << "\",\n";
        json << "      \"article_panel_id\": " << JsonStringOrNull(row.article_panel_id) << ",\n";
        json << "      \"solver_model\": \"" << EscapeJson(ToString(row.solver_model)) << "\",\n";
        json << "      \"a_over_b\": " << JsonNumber(row.a_over_b) << ",\n";
        json << "      \"article_dimension_normalized\": "
             << JsonNumber(row.article_dimension_normalized) << ",\n";
        json << "      \"computed_dimension_normalized\": "
             << JsonNumberOrNull(row.computed_dimension_normalized) << ",\n";
        json << "      \"limiting_cutoff_found\": "
             << (row.limiting_cutoff_found ? "true" : "false") << ",\n";
        json << "      \"limiting_mode_id\": " << JsonStringOrNull(row.limiting_mode_id) << ",\n";
        json << "      \"computed_b_normalized\": "
             << JsonNumberOrNull(row.computed_b_normalized) << ",\n";
        json << "      \"computed_a_normalized\": "
             << JsonNumberOrNull(row.computed_a_normalized) << ",\n";
        json << "      \"computed_b_over_A4\": "
             << JsonNumberOrNull(row.computed_b_over_A4) << ",\n";
        json << "      \"absolute_error\": " << JsonNumberOrNull(row.absolute_error) << ",\n";
        json << "      \"relative_error\": " << JsonNumberOrNull(row.relative_error) << ",\n";
        json << "      \"ex11_guided_just_below_cutoff\": "
             << (row.ex11_guided_just_below_cutoff ? "true" : "false") << ",\n";
        json << "      \"ey11_guided_just_below_cutoff\": "
             << (row.ey11_guided_just_below_cutoff ? "true" : "false") << "\n";
        json << "    }";
        if (index + 1 != result.row_summaries.size()) {
            json << ",";
        }
        json << "\n";
    }

    json << "  ]\n";
    json << "}\n";

    return json.str();
}

std::string BuildTable1SummaryCsvReport(const marcatili::Table1Result& result) {
    std::ostringstream csv;

    csv << "case_id,row_id,article_panel_id,solver_model,a_over_b,n1,n2,n3,n4,n5,"
           "article_dimension_normalized,computed_dimension_normalized,computed_b_normalized,computed_a_normalized,computed_b_over_A4,"
           "absolute_error,relative_error,limiting_cutoff_found,limiting_mode_id,"
           "limiting_mode_family,limiting_p,limiting_q,ex11_guided_just_below_cutoff,"
           "ey11_guided_just_below_cutoff\n";

    for (const auto& row : result.row_summaries) {
        csv << result.config.case_id << ","
            << row.row_id << ","
            << row.article_panel_id << ","
            << ToString(row.solver_model) << ","
            << CsvNumber(row.a_over_b) << ","
            << CsvNumber(result.config.n1) << ","
            << CsvNumber(row.n2) << ","
            << CsvNumber(row.n3) << ","
            << CsvNumber(row.n4) << ","
            << CsvNumber(row.n5) << ","
            << CsvNumber(row.article_dimension_normalized) << ","
            << CsvNumber(row.computed_dimension_normalized) << ","
            << CsvNumber(row.computed_b_normalized) << ","
            << CsvNumber(row.computed_a_normalized) << ","
            << CsvNumber(row.computed_b_over_A4) << ","
            << CsvNumber(row.absolute_error) << ","
            << CsvNumber(row.relative_error) << ","
            << (row.limiting_cutoff_found ? "1" : "0") << ","
            << row.limiting_mode_id << ","
            << ToString(row.limiting_family) << ","
            << row.limiting_p << ","
            << row.limiting_q << ","
            << (row.ex11_guided_just_below_cutoff ? "1" : "0") << ","
            << (row.ey11_guided_just_below_cutoff ? "1" : "0") << "\n";
    }

    return csv.str();
}

std::string BuildTable1DetailsCsvReport(const marcatili::Table1Result& result) {
    std::ostringstream csv;

    csv << "case_id,row_id,article_panel_id,solver_model,mode_id,mode_family,p,q,"
           "cutoff_found,cutoff_b_normalized,cutoff_a_normalized,cutoff_b_over_A4\n";

    for (const auto& cutoff : result.mode_cutoffs) {
        csv << result.config.case_id << ","
            << cutoff.row_id << ","
            << cutoff.article_panel_id << ","
            << ToString(cutoff.solver_model) << ","
            << cutoff.mode_id << ","
            << ToString(cutoff.family) << ","
            << cutoff.p << ","
            << cutoff.q << ","
            << (cutoff.cutoff_found ? "1" : "0") << ","
            << CsvNumber(cutoff.cutoff_b_normalized) << ","
            << CsvNumber(cutoff.cutoff_a_normalized) << ","
            << CsvNumber(cutoff.cutoff_b_over_A4) << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
