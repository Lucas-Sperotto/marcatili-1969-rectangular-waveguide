#pragma once

#include <string>

#include "marcatili/physics/fig6.hpp"

namespace marcatili::io {

marcatili::Figure6Config ParseFigure6Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildFigure6JsonReport(
    const marcatili::Figure6Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildFigure6CsvReport(const marcatili::Figure6Result& result);

}  // namespace marcatili::io

