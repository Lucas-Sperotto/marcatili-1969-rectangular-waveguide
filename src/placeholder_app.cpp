#include "marcatili/placeholder_app.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace marcatili {
namespace {

std::string EscapeJson(const std::string& text) {
    std::ostringstream escaped;

    for (const char ch : text) {
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
            default:
                escaped << ch;
                break;
        }
    }

    return escaped.str();
}

bool CanReadFile(const std::string& path) {
    std::ifstream input(path);
    return static_cast<bool>(input);
}

void WritePlaceholderJson(std::ostream& output,
                          const PlaceholderAppSpec& spec,
                          const std::string& input_file,
                          const std::string& output_file) {
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
        std::cerr << "Usage: " << spec.executable_name
                  << " <input_file> [output_json]\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_file = argc == 3 ? argv[2] : "";

    if (!CanReadFile(input_file)) {
        std::cerr << "Unable to open input file: " << input_file << "\n";
        return 2;
    }

    if (output_file.empty()) {
        WritePlaceholderJson(std::cout, spec, input_file, output_file);
        return 0;
    }

    std::ofstream output(output_file);
    if (!output) {
        std::cerr << "Unable to open output file: " << output_file << "\n";
        return 3;
    }

    WritePlaceholderJson(output, spec, input_file, output_file);
    std::cout << "Wrote placeholder JSON to " << output_file << "\n";
    return 0;
}

}  // namespace marcatili

