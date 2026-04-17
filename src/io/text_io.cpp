#include "marcatili/io/text_io.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace marcatili::io {

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
                // Escapa outros caracteres de controle em formato \u00XX.
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

std::string EscapeCsv(const std::string& text) {
    bool requires_quotes = false;
    std::ostringstream escaped;

    for (char ch : text) {
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
        throw std::runtime_error("ReadTextFile: unable to open input file: " + path);
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
        throw std::runtime_error("WriteTextFile: unable to open output file: " + path);
    }

    output << content;

    if (!output) {
        throw std::runtime_error("WriteTextFile: failed while writing output file: " + path);
    }
}

std::string ReplaceExtension(const std::string& path, const std::string& new_extension) {
    std::filesystem::path filesystem_path(path);
    filesystem_path.replace_extension(new_extension);
    return filesystem_path.string();
}

}  // namespace marcatili::io
