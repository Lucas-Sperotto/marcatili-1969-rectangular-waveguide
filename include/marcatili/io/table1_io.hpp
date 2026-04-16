#pragma once

#include <string>

#include "marcatili/physics/table1.hpp"

namespace marcatili::io {

marcatili::Table1Config ParseTable1Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

std::string BuildTable1JsonReport(
    const marcatili::Table1Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

std::string BuildTable1SummaryCsvReport(const marcatili::Table1Result& result);
std::string BuildTable1DetailsCsvReport(const marcatili::Table1Result& result);

}  // namespace marcatili::io
