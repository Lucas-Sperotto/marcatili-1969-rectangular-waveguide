#include "marcatili/io/table1_io.hpp"

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

std::string BuildDefaultSummaryCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".summary.csv");
}

std::string BuildDefaultDetailsCsvPath(const std::string& cli_output_json) {
    if (cli_output_json.empty()) {
        return "";
    }

    return ReplaceExtension(cli_output_json, ".details.csv");
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

marcatili::Table1RowSpec ParseTable1RowSpecLegacy(const std::string& text) {
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

    try {
        row.a_over_b = std::stod(a_over_b_text);
        row.n2 = std::stod(n2_text);
        row.n3 = std::stod(n3_text);
        row.n4 = std::stod(n4_text);
        row.n5 = std::stod(n5_text);
        row.article_dimension_normalized = std::stod(article_dimension_text);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Invalid numeric values in rows entry: " + text
        );
    }

    return row;
}

marcatili::Table1RowSpec ParseTable1RowSpecObject(const std::string& object_json) {
    marcatili::Table1RowSpec row;
    row.row_id = RequireStringValue(object_json, "row_id");
    row.article_panel_id = FindStringValue(object_json, "article_panel_id").value_or("");
    row.a_over_b = RequireDoubleValue(object_json, "a_over_b");
    row.n2 = RequireDoubleValue(object_json, "n2");
    row.n3 = RequireDoubleValue(object_json, "n3");
    row.n4 = RequireDoubleValue(object_json, "n4");
    row.n5 = RequireDoubleValue(object_json, "n5");
    row.article_dimension_normalized =
        RequireDoubleValue(object_json, "article_dimension_normalized");
    return row;
}

std::vector<marcatili::Table1RowSpec> ParseRowSpecs(const std::string& json_text) {
    std::vector<marcatili::Table1RowSpec> rows;

    const auto object_rows = FindObjectArrayValues(json_text, "rows");
    if (!object_rows.empty()) {
        rows.reserve(object_rows.size());
        for (const auto& row_object : object_rows) {
            rows.push_back(ParseTable1RowSpecObject(row_object));
        }
        return rows;
    }

    const auto string_rows = FindStringArrayValues(json_text, "rows");
    if (!string_rows.empty()) {
        rows.reserve(string_rows.size());
        for (const auto& row_text : string_rows) {
            rows.push_back(ParseTable1RowSpecLegacy(row_text));
        }
        return rows;
    }

    const auto raw_rows = FindRawJsonValue(json_text, "rows");
    if (raw_rows.has_value()) {
        throw std::runtime_error(
            "Invalid rows format. Use either an array of row objects "
            "or the legacy compact string format."
        );
    }

    throw std::runtime_error(
        "Missing required rows. Use an array of row objects "
        "or the legacy compact string format."
    );
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
            .value_or(BuildDefaultSummaryCsvPath(cli_output_json));

    config.details_csv_output_path =
        FindStringValue(json_text, "csv_details_file")
            .value_or(BuildDefaultDetailsCsvPath(cli_output_json));

    config.table_entry_interpretation =
        FindStringWithFallback(
            json_text,
            "table.table_entry_interpretation",
            "table_entry_interpretation"
        ).value_or("a_times_n1_over_lambda");

    config.wavelength =
        RequireDoubleWithFallback(json_text, "geometry.wavelength", "wavelength");
    config.n1 =
        RequireDoubleWithFallback(json_text, "materials.n1", "n1");

    config.search.max_p =
        FindIntWithFallback(json_text, "search.max_p", "search_max_p").value_or(4);
    config.search.max_q =
        FindIntWithFallback(json_text, "search.max_q", "search_max_q").value_or(4);

    config.search.b_normalized_min =
        FindDoubleWithFallback(
            json_text,
            "search.b_normalized_min",
            "search_b_normalized_min"
        ).value_or(0.01);

    config.search.b_normalized_max =
        FindDoubleWithFallback(
            json_text,
            "search.b_normalized_max",
            "search_b_normalized_max"
        ).value_or(60.0);

    config.search.cutoff_tolerance =
        FindDoubleWithFallback(
            json_text,
            "search.cutoff_tolerance",
            "search_cutoff_tolerance"
        ).value_or(1e-6);

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

    config.rows = ParseRowSpecs(json_text);

    return config;
}

std::string BuildTable1JsonReport(
    const marcatili::Table1Result& result,
    const std::string& input_file,
    const std::string& output_json_file
) {
    std::ostringstream json;
    json << "{\n";

    AppendJsonField(json, "app", "\"reproduce_table1\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(
        json,
        "output_summary_csv_file",
        JsonStringOrNull(result.config.summary_csv_output_path)
    );
    AppendJsonField(
        json,
        "output_details_csv_file",
        JsonStringOrNull(result.config.details_csv_output_path)
    );
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));
    AppendJsonField(
        json,
        "table_entry_interpretation",
        "\"" + EscapeJson(result.config.table_entry_interpretation) + "\""
    );
    AppendJsonField(
        json,
        "reference_normalization",
        JsonStringOrNull(
            "table entries multiplied by lambda / n1 and interpreted here as dimension a"
        )
    );

    json << "  \"search\": {\n";
    AppendJsonField(json, "max_p", std::to_string(result.config.search.max_p), true, 4);
    AppendJsonField(json, "max_q", std::to_string(result.config.search.max_q), true, 4);
    AppendJsonField(
        json,
        "b_normalized_min",
        JsonNumber(result.config.search.b_normalized_min),
        true,
        4
    );
    AppendJsonField(
        json,
        "b_normalized_max",
        JsonNumber(result.config.search.b_normalized_max),
        true,
        4
    );
    AppendJsonField(
        json,
        "cutoff_tolerance",
        JsonNumber(result.config.search.cutoff_tolerance),
        false,
        4
    );
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
        AppendJsonField(json, "row_id", "\"" + EscapeJson(row.row_id) + "\"", true, 6);
        AppendJsonField(json, "article_panel_id", JsonStringOrNull(row.article_panel_id), true, 6);
        AppendJsonField(
            json,
            "solver_model",
            "\"" + EscapeJson(ToString(row.solver_model)) + "\"",
            true,
            6
        );
        AppendJsonField(json, "a_over_b", JsonNumber(row.a_over_b), true, 6);
        AppendJsonField(
            json,
            "article_dimension_normalized",
            JsonNumber(row.article_dimension_normalized),
            true,
            6
        );
        AppendJsonField(
            json,
            "computed_dimension_normalized",
            JsonNumberOrNull(row.computed_dimension_normalized),
            true,
            6
        );
        AppendJsonField(
            json,
            "limiting_cutoff_found",
            row.limiting_cutoff_found ? "true" : "false",
            true,
            6
        );
        AppendJsonField(
            json,
            "limiting_mode_id",
            JsonStringOrNull(row.limiting_mode_id),
            true,
            6
        );
        AppendJsonField(
            json,
            "computed_b_normalized",
            JsonNumberOrNull(row.computed_b_normalized),
            true,
            6
        );
        AppendJsonField(
            json,
            "computed_a_normalized",
            JsonNumberOrNull(row.computed_a_normalized),
            true,
            6
        );
        AppendJsonField(
            json,
            "computed_b_over_A4",
            JsonNumberOrNull(row.computed_b_over_A4),
            true,
            6
        );
        AppendJsonField(
            json,
            "absolute_error",
            JsonNumberOrNull(row.absolute_error),
            true,
            6
        );
        AppendJsonField(
            json,
            "relative_error",
            JsonNumberOrNull(row.relative_error),
            true,
            6
        );
        AppendJsonField(
            json,
            "ex11_guided_just_below_cutoff",
            row.ex11_guided_just_below_cutoff ? "true" : "false",
            true,
            6
        );
        AppendJsonField(
            json,
            "ey11_guided_just_below_cutoff",
            row.ey11_guided_just_below_cutoff ? "true" : "false",
            false,
            6
        );
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

    csv << "case_id,table_entry_interpretation,row_id,article_panel_id,solver_model,a_over_b,n1,n2,n3,n4,n5,"
           "article_dimension_normalized,computed_dimension_normalized,computed_b_normalized,computed_a_normalized,computed_b_over_A4,"
           "absolute_error,relative_error,limiting_cutoff_found,limiting_mode_id,"
           "limiting_mode_family,limiting_p,limiting_q,ex11_guided_just_below_cutoff,"
           "ey11_guided_just_below_cutoff\n";

    for (const auto& row : result.row_summaries) {
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(result.config.table_entry_interpretation) << ","
            << EscapeCsv(row.row_id) << ","
            << EscapeCsv(row.article_panel_id) << ","
            << EscapeCsv(ToString(row.solver_model)) << ","
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
            << EscapeCsv(row.limiting_mode_id) << ","
            << EscapeCsv(ToString(row.limiting_family)) << ","
            << row.limiting_p << ","
            << row.limiting_q << ","
            << (row.ex11_guided_just_below_cutoff ? "1" : "0") << ","
            << (row.ey11_guided_just_below_cutoff ? "1" : "0") << "\n";
    }

    return csv.str();
}

std::string BuildTable1DetailsCsvReport(const marcatili::Table1Result& result) {
    std::ostringstream csv;

    csv << "case_id,table_entry_interpretation,row_id,article_panel_id,solver_model,mode_id,mode_family,p,q,"
           "cutoff_found,cutoff_b_normalized,cutoff_a_normalized,cutoff_b_over_A4\n";

    for (const auto& cutoff : result.mode_cutoffs) {
        csv << EscapeCsv(result.config.case_id) << ","
            << EscapeCsv(result.config.table_entry_interpretation) << ","
            << EscapeCsv(cutoff.row_id) << ","
            << EscapeCsv(cutoff.article_panel_id) << ","
            << EscapeCsv(ToString(cutoff.solver_model)) << ","
            << EscapeCsv(cutoff.mode_id) << ","
            << EscapeCsv(ToString(cutoff.family)) << ","
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
