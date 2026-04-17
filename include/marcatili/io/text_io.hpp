#pragma once

#include <string>

namespace marcatili::io {

std::string EscapeJson(const std::string& text);
std::string EscapeCsv(const std::string& text);
std::string ReadTextFile(const std::string& path);
void WriteTextFile(const std::string& path, const std::string& content);
std::string ReplaceExtension(const std::string& path, const std::string& new_extension);

}  // namespace marcatili::io
