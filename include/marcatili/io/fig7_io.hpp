#pragma once

#include <string>

#include "marcatili/physics/fig7.hpp"

namespace marcatili::io {

marcatili::Figure7Config ParseFigure7Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildFigure7JsonReport(
    const marcatili::Figure7Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildFigure7LinesCsvReport(const marcatili::Figure7Result& result);
std::string BuildFigure7IntersectionsCsvReport(const marcatili::Figure7Result& result);

}  // namespace marcatili::io
