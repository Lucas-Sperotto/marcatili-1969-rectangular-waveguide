#include "marcatili/io/sweep_index_io.hpp"

#include <algorithm>
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

std::string CsvNumber(double value) {
    if (!std::isfinite(value)) {
        return "nan";
    }

    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
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

std::vector<double> ParseDoubleArray(const std::string& raw_json, const std::string& key_name) {
    const std::string trimmed = TrimWhitespaceCopy(raw_json);
    if (trimmed.size() < 2 || trimmed.front() != '[' || trimmed.back() != ']') {
        throw std::runtime_error("ParseSweepIndexConfig: " + key_name + " must be an array.");
    }

    const std::string body = trimmed.substr(1, trimmed.size() - 2);
    std::stringstream parser(body);
    std::string item;
    std::vector<double> values;

    while (std::getline(parser, item, ',')) {
        const std::string token = TrimWhitespaceCopy(item);
        if (token.empty()) {
            throw std::runtime_error(
                "ParseSweepIndexConfig: " + key_name + " contains an empty value."
            );
        }

        std::size_t parsed_characters = 0;
        try {
            const double value = std::stod(token, &parsed_characters);
            if (parsed_characters != token.size() || !std::isfinite(value)) {
                throw std::runtime_error("invalid value");
            }
            values.push_back(value);
        } catch (const std::exception&) {
            throw std::runtime_error(
                "ParseSweepIndexConfig: invalid numeric value in " + key_name + ": " + token
            );
        }
    }

    return values;
}

std::vector<double> RequireDoubleArrayValues(
    const std::string& json_text,
    const std::string& key_name
) {
    const auto raw_value = FindRawJsonValue(json_text, key_name);
    if (!raw_value.has_value()) {
        throw std::runtime_error("Missing required numeric-array key: " + key_name);
    }

    return ParseDoubleArray(*raw_value, key_name);
}

SweepIndexModeSpec ParseSweepIndexModeSpec(const std::string& mode_text) {
    std::stringstream parser(mode_text);
    std::string family_text;
    std::string p_text;
    std::string q_text;

    if (!std::getline(parser, family_text, ':') ||
        !std::getline(parser, p_text, ':') ||
        !std::getline(parser, q_text, ':')) {
        throw std::invalid_argument(
            "ParseSweepIndexModeSpec: invalid mode specification '" + mode_text +
            "'. Use family:p:q, for example E_y:1:1."
        );
    }

    SweepIndexModeSpec mode;
    mode.family = marcatili::ParseSingleGuideFamily(family_text);

    try {
        mode.p = std::stoi(p_text);
        mode.q = std::stoi(q_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseSweepIndexModeSpec: invalid numeric indices in '" + mode_text + "'."
        );
    }

    if (mode.p <= 0 || mode.q <= 0) {
        throw std::invalid_argument(
            "ParseSweepIndexModeSpec: mode indices must be positive in '" +
            mode_text + "'."
        );
    }

    return mode;
}

void ValidateBaseCase(const SweepIndexBaseCase& base_case) {
    if (!std::isfinite(base_case.a) || base_case.a <= 0.0 ||
        !std::isfinite(base_case.b) || base_case.b <= 0.0 ||
        !std::isfinite(base_case.wavelength) || base_case.wavelength <= 0.0) {
        throw std::invalid_argument(
            "ParseSweepIndexConfig: base_case a, b and wavelength must be positive."
        );
    }

    const double external_max =
        std::max(std::max(base_case.n2, base_case.n3), std::max(base_case.n4, base_case.n5));

    if (!std::isfinite(base_case.n1) || base_case.n1 <= 0.0 ||
        !std::isfinite(base_case.n2) || base_case.n2 <= 0.0 ||
        !std::isfinite(base_case.n3) || base_case.n3 <= 0.0 ||
        !std::isfinite(base_case.n4) || base_case.n4 <= 0.0 ||
        !std::isfinite(base_case.n5) || base_case.n5 <= 0.0 ||
        !(base_case.n1 > external_max)) {
        throw std::invalid_argument(
            "ParseSweepIndexConfig: base_case requires positive indices with n1 greater than n2..n5."
        );
    }
}

void ValidateConfig(const SweepIndexConfig& config) {
    if (config.case_id.empty()) {
        throw std::invalid_argument("ParseSweepIndexConfig: case_id must not be empty.");
    }

    if (config.solver_model != marcatili::SingleGuideSolverModel::kExact) {
        throw std::invalid_argument(
            "ParseSweepIndexConfig: reproduce_sweep_index only supports solver_model exact."
        );
    }

    if (config.sweep_parameter != "n4") {
        throw std::invalid_argument(
            "ParseSweepIndexConfig: only sweep.parameter = \"n4\" is supported."
        );
    }

    ValidateBaseCase(config.base_case);

    if (config.sweep_values.empty()) {
        throw std::invalid_argument("ParseSweepIndexConfig: sweep.values must not be empty.");
    }

    for (const double value : config.sweep_values) {
        if (!std::isfinite(value) || value <= 0.0 || !(config.base_case.n1 > value)) {
            throw std::invalid_argument(
                "ParseSweepIndexConfig: each n4 sweep value must be positive and less than n1."
            );
        }
    }

    if (config.modes.empty()) {
        throw std::invalid_argument("ParseSweepIndexConfig: at least one mode is required.");
    }
}

}  // namespace

SweepIndexConfig ParseSweepIndexConfig(const std::string& json_text) {
    SweepIndexConfig config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");

    config.solver_model = marcatili::ParseSingleGuideSolverModel(
        FindStringValue(json_text, "solver_model").value_or("exact")
    );

    config.base_case.a = RequireDoubleValue(json_text, "base_case.a");
    config.base_case.b = RequireDoubleValue(json_text, "base_case.b");
    config.base_case.wavelength = RequireDoubleValue(json_text, "base_case.wavelength");
    config.base_case.n1 = RequireDoubleValue(json_text, "base_case.n1");
    config.base_case.n2 = RequireDoubleValue(json_text, "base_case.n2");
    config.base_case.n3 = RequireDoubleValue(json_text, "base_case.n3");
    config.base_case.n4 = RequireDoubleValue(json_text, "base_case.n4");
    config.base_case.n5 = RequireDoubleValue(json_text, "base_case.n5");

    config.sweep_parameter = RequireStringValue(json_text, "sweep.parameter");
    config.sweep_values = RequireDoubleArrayValues(json_text, "sweep.values");

    for (const auto& mode_text : RequireStringArrayValues(json_text, "modes")) {
        config.modes.push_back(ParseSweepIndexModeSpec(mode_text));
    }

    ValidateConfig(config);
    return config;
}

std::string BuildSweepIndexCsvReport(const std::vector<SweepIndexSample>& samples) {
    std::ostringstream csv;

    csv << "n4,mode_family,p,q,kz,ky,kx\n";

    for (const auto& sample : samples) {
        csv << CsvNumber(sample.n4) << ","
            << EscapeCsv(marcatili::ToString(sample.family)) << ","
            << sample.p << ","
            << sample.q << ","
            << CsvNumber(sample.kz) << ","
            << CsvNumber(sample.ky) << ","
            << CsvNumber(sample.kx) << "\n";
    }

    return csv.str();
}

}  // namespace marcatili::io
