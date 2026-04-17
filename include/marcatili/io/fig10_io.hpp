#pragma once

#include <string>

#include "marcatili/physics/fig10.hpp"

namespace marcatili::io {

marcatili::Figure10Config ParseFigure10Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildFigure10JsonReport(
    const marcatili::Figure10Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildFigure10CsvReport(const marcatili::Figure10Result& result);

}  // namespace marcatili::io
