#include "marcatili/placeholder_app.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace marcatili {
namespace {

enum class ExitCode : int {
    kSuccess = 0,
    kUsageError = 1,
    kInputOpenError = 2,
    kOutputOpenError = 3
};

void WriteUsage(std::ostream& output, const PlaceholderAppSpec& spec) {
    output << "Usage: " << spec.executable_name
           << " <input_file> [output_json]\n";
}

std::string EscapeJson(const std::string& text) {
    std::ostringstream escaped;

    for (unsigned char ch : text) {
        switch (ch) {
            case '\\':
                escaped << "\\\\";
                break;
            case '"':
                escaped << "\\\"";
                break;
            case '\n':
                escaped << "\\n";
                break;
            case '\r':
                escaped << "\\r";
                break;
            case '\t':
                escaped << "\\t";
                break;
            case '\b':
                escaped << "\\b";
                break;
            case '\f':
                escaped << "\\f";
                break;
            default:
                // Escapa caracteres de controle restantes no formato \u00XX.
                if (ch < 0x20) {
                    escaped << "\\u"
                            << std::hex
                            << std::uppercase
                            << std::setw(4)
                            << std::setfill('0')
                            << static_cast<int>(ch)
                            << std::dec
                            << std::nouppercase;
                } else {
                    escaped << static_cast<char>(ch);
                }
                break;
        }
    }

    return escaped.str();
}

bool CanReadFile(const std::string& path) {
    std::ifstream input(path);
    return static_cast<bool>(input);
}

void WritePlaceholderJson(
    std::ostream& output,
    const PlaceholderAppSpec& spec,
    const std::string& input_file,
    const std::string& output_file
) {
    output << "{\n";
    output << "  \"app\": \"" << EscapeJson(spec.executable_name) << "\",\n";
    output << "  \"status\": \"placeholder\",\n";
    output << "  \"objective\": \"" << EscapeJson(spec.objective) << "\",\n";
    output << "  \"input_file\": \"" << EscapeJson(input_file) << "\",\n";
    output << "  \"output_file\": ";

    if (output_file.empty()) {
        output << "null,\n";
    } else {
        output << "\"" << EscapeJson(output_file) << "\",\n";
    }

    output << "  \"message\": "
           << "\"Mathematical model not implemented yet; repository scaffold is ready for future steps.\",\n";
    output << "  \"next_step\": "
           << "\"Replace this placeholder with the equations, validation cases, and article references for this target.\"\n";
    output << "}\n";
}

}  // namespace

int RunPlaceholderApp(const PlaceholderAppSpec& spec, int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        WriteUsage(std::cerr, spec);
        return static_cast<int>(ExitCode::kUsageError);
    }

    const std::string input_file = argv[1];
    const std::string output_file = (argc == 3) ? argv[2] : "";

    if (!CanReadFile(input_file)) {
        std::cerr << "Unable to open input file: " << input_file << "\n";
        return static_cast<int>(ExitCode::kInputOpenError);
    }

    if (output_file.empty()) {
        WritePlaceholderJson(std::cout, spec, input_file, output_file);
        return static_cast<int>(ExitCode::kSuccess);
    }

    std::ofstream output(output_file);
    if (!output) {
        std::cerr << "Unable to open output file: " << output_file << "\n";
        return static_cast<int>(ExitCode::kOutputOpenError);
    }

    WritePlaceholderJson(output, spec, input_file, output_file);
    std::cout << "Wrote placeholder JSON to " << output_file << "\n";

    return static_cast<int>(ExitCode::kSuccess);
}

}  // namespace marcatili
