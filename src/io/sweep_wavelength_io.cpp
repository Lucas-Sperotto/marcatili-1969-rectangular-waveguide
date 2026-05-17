#include "marcatili/io/sweep_wavelength_io.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <optional>
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

SweepWavelengthModeSpec ParseSweepWavelengthModeSpec(const std::string& mode_text) {
    std::stringstream parser(mode_text);
    std::string family_text;
    std::string p_text;
    std::string q_text;

    if (!std::getline(parser, family_text, ':') ||
        !std::getline(parser, p_text, ':') ||
        !std::getline(parser, q_text, ':')) {
        throw std::invalid_argument(
            "ParseSweepWavelengthModeSpec: invalid mode specification '" + mode_text +
            "'. Use family:p:q, for example E_y:1:1."
        );
    }

    SweepWavelengthModeSpec mode;
    mode.family = marcatili::ParseSingleGuideFamily(family_text);

    try {
        mode.p = std::stoi(p_text);
        mode.q = std::stoi(q_text);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "ParseSweepWavelengthModeSpec: invalid numeric indices in '" +
            mode_text + "'."
        );
    }

    if (mode.p <= 0 || mode.q <= 0) {
        throw std::invalid_argument(
            "ParseSweepWavelengthModeSpec: mode indices must be positive in '" +
            mode_text + "'."
        );
    }

    return mode;
}

void ValidateConfig(const SweepWavelengthConfig& config) {
    if (config.case_id.empty()) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: case_id must not be empty."
        );
    }

    if (config.solver_model != marcatili::SingleGuideSolverModel::kExact) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: reproduce_sweep_wavelength only supports solver_model exact."
        );
    }

    if (!std::isfinite(config.a) || config.a <= 0.0 ||
        !std::isfinite(config.b) || config.b <= 0.0) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: a and b must be positive."
        );
    }

    const double external_max =
        std::max(std::max(config.n2, config.n3), std::max(config.n4, config.n5));

    if (!std::isfinite(config.n1) || config.n1 <= 0.0 ||
        !std::isfinite(config.n2) || config.n2 <= 0.0 ||
        !std::isfinite(config.n3) || config.n3 <= 0.0 ||
        !std::isfinite(config.n4) || config.n4 <= 0.0 ||
        !std::isfinite(config.n5) || config.n5 <= 0.0 ||
        !(config.n1 > external_max)) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: requires positive indices with n1 greater than n2..n5."
        );
    }

    if (!std::isfinite(config.min_wavelength) || config.min_wavelength <= 0.0 ||
        !std::isfinite(config.max_wavelength) || config.max_wavelength <= 0.0) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: wavelength sweep bounds must be positive."
        );
    }

    if (config.max_wavelength <= config.min_wavelength) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: max_wavelength must be greater than min_wavelength."
        );
    }

    if (config.point_count < 2) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: point_count must be at least 2."
        );
    }

    if (config.modes.empty()) {
        throw std::invalid_argument(
            "ParseSweepWavelengthConfig: at least one mode is required."
        );
    }
}

}  // namespace

SweepWavelengthConfig ParseSweepWavelengthConfig(const std::string& json_text) {
    SweepWavelengthConfig config;

    config.case_id = RequireStringValue(json_text, "case_id");
    config.article_target = FindStringValue(json_text, "article_target").value_or("");

    config.solver_model = marcatili::ParseSingleGuideSolverModel(
        FindStringValue(json_text, "solver_model").value_or("exact")
    );

    config.a = RequireDoubleWithFallback(json_text, "geometry.a", "a");
    config.b = RequireDoubleWithFallback(json_text, "geometry.b", "b");

    config.n1 = RequireDoubleWithFallback(json_text, "materials.n1", "n1");
    config.n2 = RequireDoubleWithFallback(json_text, "materials.n2", "n2");
    config.n3 = RequireDoubleWithFallback(json_text, "materials.n3", "n3");
    config.n4 = RequireDoubleWithFallback(json_text, "materials.n4", "n4");
    config.n5 = RequireDoubleWithFallback(json_text, "materials.n5", "n5");

    config.min_wavelength =
        RequireDoubleWithFallback(json_text, "sweep.min_wavelength", "min_wavelength");
    config.max_wavelength =
        RequireDoubleWithFallback(json_text, "sweep.max_wavelength", "max_wavelength");
    config.point_count =
        RequireIntWithFallback(json_text, "sweep.point_count", "point_count");

    for (const auto& mode_text : RequireStringArrayValues(json_text, "modes")) {
        config.modes.push_back(ParseSweepWavelengthModeSpec(mode_text));
    }

    ValidateConfig(config);
    return config;
}

std::string BuildSweepWavelengthCsvReport(
    const std::vector<SweepWavelengthSample>& samples
) {
    std::ostringstream csv;

    csv << "wavelength,mode_family,p,q,kz_rad_per_m,ky,kx\n";

    for (const auto& sample : samples) {
        csv << CsvNumber(sample.wavelength) << ","
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
