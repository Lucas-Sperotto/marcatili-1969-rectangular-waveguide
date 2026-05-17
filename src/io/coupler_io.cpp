#include "marcatili/io/coupler_io.hpp"

#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include "marcatili/io/schema_json.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/math/waveguide_math.hpp"

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

std::optional<double> FindGuideDouble(
    const std::string& json_text,
    const std::string& guide_key,
    const std::string& field_name
) {
    const auto flat_value = FindDoubleValue(json_text, guide_key + "." + field_name);
    if (flat_value.has_value()) {
        return flat_value;
    }

    const auto material_value =
        FindDoubleValue(json_text, guide_key + ".materials." + field_name);
    if (material_value.has_value()) {
        return material_value;
    }

    return FindDoubleValue(json_text, guide_key + ".geometry." + field_name);
}

double RequireGuideDouble(
    const std::string& json_text,
    const std::string& guide_key,
    const std::string& field_name
) {
    const auto value = FindGuideDouble(json_text, guide_key, field_name);
    if (!value.has_value()) {
        throw std::runtime_error(
            "Missing required numeric key in " + guide_key + ": " + field_name
        );
    }

    return *value;
}

bool HasPerturbedGuideObjects(const std::string& json_text) {
    const bool has_guide_1 = FindRawJsonValue(json_text, "guide_1").has_value();
    const bool has_guide_2 = FindRawJsonValue(json_text, "guide_2").has_value();

    if (has_guide_1 != has_guide_2) {
        throw std::runtime_error(
            "Perturbed coupler input must provide both guide_1 and guide_2."
        );
    }

    return has_guide_1 && has_guide_2;
}

marcatili::CouplerGuideConfig ParseCouplerGuide(
    const std::string& json_text,
    const std::string& guide_key
) {
    marcatili::CouplerGuideConfig guide;
    guide.a = RequireGuideDouble(json_text, guide_key, "a");
    guide.b = RequireGuideDouble(json_text, guide_key, "b");
    guide.n1 = RequireGuideDouble(json_text, guide_key, "n1");
    guide.n2 = RequireGuideDouble(json_text, guide_key, "n2");
    guide.n3 = RequireGuideDouble(json_text, guide_key, "n3");
    guide.n4 = RequireGuideDouble(json_text, guide_key, "n4");
    guide.n5 = RequireGuideDouble(json_text, guide_key, "n5");
    return guide;
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

marcatili::CouplerPointConfig ParseCouplerPointConfig(
    const std::string& json_text,
    const std::string& cli_output_json
) {
    marcatili::CouplerPointConfig config;
    const bool has_perturbed_guides = HasPerturbedGuideObjects(json_text);

    const auto case_id = FindStringValue(json_text, "case_id");
    if (case_id.has_value()) {
        config.case_id = *case_id;
    } else {
        config.case_id =
            FindStringValue(json_text, "case_name").value_or("CP-POINT-UNSPECIFIED");
    }

    const auto article_target = FindStringValue(json_text, "article_target");
    if (article_target.has_value()) {
        config.article_target = *article_target;
    } else {
        config.article_target =
            FindStringValue(json_text, "article_scope").value_or("");
    }

    config.csv_output_path =
        FindStringValue(json_text, "csv_file")
            .value_or(BuildDefaultCsvPath(cli_output_json));

    config.solver_model = marcatili::ParseSingleGuideSolverModel(
        FindStringValue(json_text, "solver_model").value_or("exact")
    );

    config.transverse_equation = marcatili::ParseCouplerTransverseEquation(
        FindStringValue(json_text, "transverse_equation").value_or("eq6")
    );

    config.p = FindIntWithFallback(json_text, "mode_indices.p", "p").value_or(1);
    config.q = FindIntWithFallback(json_text, "mode_indices.q", "q").value_or(1);

    config.wavelength =
        FindDoubleWithFallback(json_text, "geometry.wavelength", "wavelength").value_or(0.0);

    if (has_perturbed_guides) {
        config.perturbed_guides_enabled = true;
        config.guide_1 = ParseCouplerGuide(json_text, "guide_1");
        config.guide_2 = ParseCouplerGuide(json_text, "guide_2");

        if (!(config.wavelength > 0.0)) {
            throw std::runtime_error(
                "Perturbed coupler input requires geometry.wavelength."
            );
        }
    }

    const auto a_over_A5 =
        FindDoubleWithFallback(json_text, "normalized_geometry.a_over_A5", "a_over_A5");
    if (a_over_A5.has_value()) {
        config.a_over_A5 = *a_over_A5;
    } else if (has_perturbed_guides) {
        const double A5 = marcatili::math::ComputeA(
            config.wavelength,
            config.guide_1.n1,
            config.guide_1.n5
        );
        config.a_over_A5 = config.guide_1.a / A5;
    } else {
        config.a_over_A5 =
            RequireDoubleWithFallback(json_text, "normalized_geometry.a_over_A5", "a_over_A5");
    }

    const auto c_over_a =
        FindDoubleWithFallback(json_text, "normalized_geometry.c_over_a", "c_over_a");
    const auto c =
        FindDoubleWithFallback(json_text, "geometry.c", "c");
    if (c_over_a.has_value()) {
        config.c_over_a = *c_over_a;
    } else if (has_perturbed_guides && c.has_value()) {
        config.c_over_a = *c / config.guide_1.a;
    } else {
        config.c_over_a =
            RequireDoubleWithFallback(json_text, "normalized_geometry.c_over_a", "c_over_a");
    }

    const auto index_ratio_squared =
        FindDoubleWithFallback(json_text, "materials.index_ratio_squared", "index_ratio_squared");
    const auto n1_over_n5 =
        FindDoubleWithFallback(json_text, "materials.n1_over_n5", "n1_over_n5");

    if (index_ratio_squared.has_value()) {
        config.index_ratio_squared = *index_ratio_squared;
    } else if (n1_over_n5.has_value() && *n1_over_n5 > 0.0) {
        config.index_ratio_squared = 1.0 / (*n1_over_n5 * *n1_over_n5);
    } else {
        config.index_ratio_squared = 0.0;
    }

    config.n1 = FindDoubleWithFallback(json_text, "materials.n1", "n1").value_or(0.0);
    config.n5 = FindDoubleWithFallback(json_text, "materials.n5", "n5").value_or(0.0);

    if (has_perturbed_guides) {
        if (!(config.n1 > 0.0)) {
            config.n1 = config.guide_1.n1;
        }
        if (!(config.n5 > 0.0)) {
            config.n5 = config.guide_1.n5;
        }
    }

    if (!(config.index_ratio_squared > 0.0) &&
        config.n1 > 0.0 &&
        config.n5 > 0.0 &&
        config.n1 > config.n5) {
        config.index_ratio_squared = (config.n5 * config.n5) / (config.n1 * config.n1);
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

    AppendJsonField(json, "app", "\"solve_coupler\"");
    AppendJsonField(json, "status", "\"" + EscapeJson(result.status) + "\"");
    AppendJsonField(json, "status_class", "\"" + EscapeJson(result.status_class) + "\"");
    AppendJsonField(
        json,
        "model",
        "\"" + EscapeJson(ToString(result.config.solver_model)) + "\""
    );
    AppendJsonField(
        json,
        "transverse_equation",
        "\"" + EscapeJson(ToString(result.config.transverse_equation)) + "\""
    );
    AppendJsonField(json, "equations_used", JsonStringOrNull(result.equations_used));
    AppendJsonField(json, "input_file", JsonStringOrNull(input_file));
    AppendJsonField(json, "output_json_file", JsonStringOrNull(output_json_file));
    AppendJsonField(json, "output_csv_file", JsonStringOrNull(result.config.csv_output_path));
    AppendJsonField(json, "case_id", "\"" + EscapeJson(result.config.case_id) + "\"");
    AppendJsonField(json, "article_target", JsonStringOrNull(result.config.article_target));
    AppendJsonField(json, "domain_valid", result.domain_valid ? "true" : "false");
    AppendJsonField(
        json,
        "transverse_root_found",
        result.transverse_root_found ? "true" : "false"
    );
    AppendJsonField(
        json,
        "dimensional_outputs_available",
        result.dimensional_outputs_available ? "true" : "false"
    );

    json << "  \"normalized_inputs\": {\n";
    AppendJsonField(json, "p", std::to_string(result.config.p), true, 4);
    AppendJsonField(json, "q", std::to_string(result.config.q), true, 4);
    AppendJsonField(json, "a_over_A5", JsonNumber(result.config.a_over_A5), true, 4);
    AppendJsonField(json, "c_over_a", JsonNumber(result.config.c_over_a), true, 4);
    AppendJsonField(
        json,
        "index_ratio_squared",
        JsonNumberOrNull(result.config.index_ratio_squared),
        false,
        4
    );
    json << "  },\n";

    json << "  \"normalized_outputs\": {\n";
    AppendJsonField(json, "a_over_A5", JsonNumberOrNull(result.a_over_A5), true, 4);
    AppendJsonField(json, "c_over_A5", JsonNumberOrNull(result.c_over_A5), true, 4);
    AppendJsonField(json, "kx_A5_over_pi", JsonNumberOrNull(result.kx_A5_over_pi), true, 4);
    AppendJsonField(
        json,
        "sqrt_one_minus_kx_A5_over_pi_squared",
        JsonNumberOrNull(result.sqrt_one_minus_kx_A5_over_pi_squared),
        true,
        4
    );
    AppendJsonField(
        json,
        "normalized_coupling",
        JsonNumberOrNull(result.normalized_coupling),
        false,
        4
    );
    json << "  },\n";

    json << "  \"dimensional_inputs\": {\n";
    AppendJsonField(json, "wavelength", JsonNumberOrNull(result.config.wavelength), true, 4);
    AppendJsonField(json, "n1", JsonNumberOrNull(result.config.n1), true, 4);
    AppendJsonField(json, "n5", JsonNumberOrNull(result.config.n5), false, 4);
    json << "  },\n";

    json << "  \"dimensional_outputs\": {\n";
    AppendJsonField(json, "A5", JsonNumberOrNull(result.A5), true, 4);
    AppendJsonField(json, "a", JsonNumberOrNull(result.a), true, 4);
    AppendJsonField(json, "c", JsonNumberOrNull(result.c), true, 4);
    AppendJsonField(json, "k0", JsonNumberOrNull(result.k0), true, 4);
    AppendJsonField(json, "k1", JsonNumberOrNull(result.k1), true, 4);
    AppendJsonField(json, "k5", JsonNumberOrNull(result.k5), true, 4);
    AppendJsonField(json, "kx", JsonNumberOrNull(result.kx), true, 4);
    AppendJsonField(json, "kz", JsonNumberOrNull(result.kz), true, 4);
    AppendJsonField(
        json,
        "coupling_magnitude",
        JsonNumberOrNull(result.coupling_magnitude),
        true,
        4
    );
    AppendJsonField(
        json,
        "full_transfer_length",
        JsonNumberOrNull(result.full_transfer_length),
        false,
        4
    );
    json << "  },\n";

    json << "  \"perturbed_outputs\": {\n";
    AppendJsonField(
        json,
        "available",
        result.perturbed_outputs_available ? "true" : "false",
        true,
        4
    );
    AppendJsonField(json, "beta_1", JsonNumberOrNull(result.beta_1), true, 4);
    AppendJsonField(json, "beta_2", JsonNumberOrNull(result.beta_2), true, 4);
    AppendJsonField(json, "delta", JsonNumberOrNull(result.delta), true, 4);
    AppendJsonField(
        json,
        "effective_coupling_magnitude",
        JsonNumberOrNull(result.effective_coupling_magnitude),
        false,
        4
    );
    json << "  },\n";

    AppendJsonField(
        json,
        "note",
        JsonStringOrNull(
            "This executable always reports the normalized Eq. (34) model. "
            "When wavelength, n1 and n5 are also provided, it additionally reconstructs "
            "A5, a, c, |K| and L using the same reduced transverse model."
        ),
        false
    );

    json << "}\n";
    return json.str();
}

std::string BuildCouplerPointCsvReport(const marcatili::CouplerPointResult& result) {
    std::ostringstream csv;

    csv << "case_id,solver_model,transverse_equation,p,a_over_A5,c_over_a,c_over_A5,"
           "index_ratio_squared,wavelength,n1,n5,transverse_root_found,dimensional_outputs_available,"
           "kx_A5_over_pi,sqrt_one_minus_kx_A5_over_pi_squared,normalized_coupling,"
           "A5,a,c,k0,k1,k5,kx,kz,coupling_magnitude,full_transfer_length,"
           "domain_valid,status,status_class,equations_used,"
           "q,perturbed_outputs_available,beta_1,beta_2,delta,effective_coupling_magnitude\n";

    csv << EscapeCsv(result.config.case_id) << ","
        << EscapeCsv(ToString(result.config.solver_model)) << ","
        << EscapeCsv(ToString(result.config.transverse_equation)) << ","
        << result.config.p << ","
        << CsvNumber(result.config.a_over_A5) << ","
        << CsvNumber(result.config.c_over_a) << ","
        << CsvNumber(result.c_over_A5) << ","
        << CsvNumber(result.config.index_ratio_squared) << ","
        << CsvNumber(result.config.wavelength) << ","
        << CsvNumber(result.config.n1) << ","
        << CsvNumber(result.config.n5) << ","
        << (result.transverse_root_found ? "1" : "0") << ","
        << (result.dimensional_outputs_available ? "1" : "0") << ","
        << CsvNumber(result.kx_A5_over_pi) << ","
        << CsvNumber(result.sqrt_one_minus_kx_A5_over_pi_squared) << ","
        << CsvNumber(result.normalized_coupling) << ","
        << CsvNumber(result.A5) << ","
        << CsvNumber(result.a) << ","
        << CsvNumber(result.c) << ","
        << CsvNumber(result.k0) << ","
        << CsvNumber(result.k1) << ","
        << CsvNumber(result.k5) << ","
        << CsvNumber(result.kx) << ","
        << CsvNumber(result.kz) << ","
        << CsvNumber(result.coupling_magnitude) << ","
        << CsvNumber(result.full_transfer_length) << ","
        << (result.domain_valid ? "1" : "0") << ","
        << EscapeCsv(result.status) << ","
        << EscapeCsv(result.status_class) << ","
        << EscapeCsv(result.equations_used) << ","
        << result.config.q << ","
        << (result.perturbed_outputs_available ? "1" : "0") << ","
        << CsvNumber(result.beta_1) << ","
        << CsvNumber(result.beta_2) << ","
        << CsvNumber(result.delta) << ","
        << CsvNumber(result.effective_coupling_magnitude) << "\n";

    return csv.str();
}

}  // namespace marcatili::io
