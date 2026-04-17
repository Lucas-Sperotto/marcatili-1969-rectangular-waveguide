#pragma once

#include <string>

#include "marcatili/physics/coupler.hpp"

namespace marcatili::io {

marcatili::CouplerPointConfig ParseCouplerPointConfig(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildCouplerPointJsonReport(
    const marcatili::CouplerPointResult& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildCouplerPointCsvReport(const marcatili::CouplerPointResult& result);

}  // namespace marcatili::io
