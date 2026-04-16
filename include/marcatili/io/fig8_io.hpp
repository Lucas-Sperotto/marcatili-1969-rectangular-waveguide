#pragma once

#include <string>

#include "marcatili/physics/fig8.hpp"

namespace marcatili::io {

marcatili::Figure8Config ParseFigure8Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildFigure8JsonReport(
    const marcatili::Figure8Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildFigure8CsvReport(const marcatili::Figure8Result& result);

}  // namespace marcatili::io
