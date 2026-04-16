#pragma once

#include <string>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili::io {

marcatili::SingleGuideConfig ParseSingleGuideConfig(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildSingleGuideJsonReport(
    const marcatili::SingleGuideResult& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildSingleGuideCsvReport(const marcatili::SingleGuideResult& result);

}  // namespace marcatili::io

