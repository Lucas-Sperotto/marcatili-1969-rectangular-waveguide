#pragma once

#include <string>

#include "marcatili/physics/fig11.hpp"

namespace marcatili::io {

marcatili::Figure11Config ParseFigure11Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildFigure11JsonReport(
    const marcatili::Figure11Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildFigure11CsvReport(const marcatili::Figure11Result& result);

}  // namespace marcatili::io
