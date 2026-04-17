#include "marcatili/io/text_io.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace marcatili::io {

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

std::string EscapeCsv(const std::string& text) {
    bool requires_quotes = false;
    std::ostringstream escaped;

    for (const char ch : text) {
        if (ch == '"' || ch == ',' || ch == '\n' || ch == '\r') {
            requires_quotes = true;
        }

        if (ch == '"') {
            escaped << "\"\"";
        } else {
            escaped << ch;
        }
    }

    if (!requires_quotes) {
        return escaped.str();
    }

    return "\"" + escaped.str() + "\"";
}

std::string ReadTextFile(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open input file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void WriteTextFile(const std::string& path, const std::string& content) {
    const std::filesystem::path filesystem_path(path);
    if (filesystem_path.has_parent_path()) {
        std::filesystem::create_directories(filesystem_path.parent_path());
    }

    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Unable to open output file: " + path);
    }

    output << content;
}

std::string ReplaceExtension(const std::string& path, const std::string& new_extension) {
    std::filesystem::path filesystem_path(path);
    filesystem_path.replace_extension(new_extension);
    return filesystem_path.string();
}

}  // namespace marcatili::io
