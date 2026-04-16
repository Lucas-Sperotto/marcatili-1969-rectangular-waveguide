#include "marcatili/io/schema_json.hpp"

#include <regex>
#include <stdexcept>

namespace marcatili::io {
namespace {

std::string EscapeRegex(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size() * 2);

    for (const char ch : text) {
        switch (ch) {
            case '\\':
            case '^':
            case '$':
            case '.':
            case '|':
            case '?':
            case '*':
            case '+':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
                escaped.push_back('\\');
                escaped.push_back(ch);
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }

    return escaped;
}

std::optional<std::string> FindMatch(
    const std::string& json_text,
    const std::string& pattern
) {
    const std::regex regex_pattern(pattern, std::regex::ECMAScript);
    std::smatch match;

    if (!std::regex_search(json_text, match, regex_pattern)) {
        return std::nullopt;
    }

    return match[1].str();
}

}  // namespace

std::optional<std::string> FindStringValue(
    const std::string& json_text,
    const std::string& key
) {
    const std::string escaped_key = EscapeRegex(key);
    const std::string pattern =
        "\"" + escaped_key + "\"\\s*:\\s*\"([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)\"";
    return FindMatch(json_text, pattern);
}

std::optional<double> FindDoubleValue(
    const std::string& json_text,
    const std::string& key
) {
    const std::string escaped_key = EscapeRegex(key);
    const std::string pattern =
        "\"" + escaped_key + "\"\\s*:\\s*"
        "([-+]?(?:\\d+\\.?\\d*|\\.\\d+)(?:[eE][-+]?\\d+)?)";

    const auto match = FindMatch(json_text, pattern);
    if (!match.has_value()) {
        return std::nullopt;
    }

    return std::stod(*match);
}

std::optional<int> FindIntValue(
    const std::string& json_text,
    const std::string& key
) {
    const std::string escaped_key = EscapeRegex(key);
    const std::string pattern = "\"" + escaped_key + "\"\\s*:\\s*(-?\\d+)";

    const auto match = FindMatch(json_text, pattern);
    if (!match.has_value()) {
        return std::nullopt;
    }

    return std::stoi(*match);
}

std::vector<std::string> FindStringArrayValues(
    const std::string& json_text,
    const std::string& key
) {
    const std::string escaped_key = EscapeRegex(key);
    const std::string array_pattern =
        "\"" + escaped_key + "\"\\s*:\\s*\\[([\\s\\S]*?)\\]";

    const auto array_body = FindMatch(json_text, array_pattern);
    if (!array_body.has_value()) {
        return {};
    }

    const std::regex item_pattern(
        "\"([^\"\\\\]*(?:\\\\.[^\"\\\\]*)*)\"",
        std::regex::ECMAScript
    );

    std::vector<std::string> values;
    auto begin = std::sregex_iterator(array_body->begin(), array_body->end(), item_pattern);
    const auto end = std::sregex_iterator();

    for (auto iterator = begin; iterator != end; ++iterator) {
        values.push_back((*iterator)[1].str());
    }

    return values;
}

std::string RequireStringValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto value = FindStringValue(json_text, key);
    if (!value.has_value()) {
        throw std::runtime_error("Missing required string key: " + key);
    }

    return *value;
}

double RequireDoubleValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto value = FindDoubleValue(json_text, key);
    if (!value.has_value()) {
        throw std::runtime_error("Missing required numeric key: " + key);
    }

    return *value;
}

int RequireIntValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto value = FindIntValue(json_text, key);
    if (!value.has_value()) {
        throw std::runtime_error("Missing required integer key: " + key);
    }

    return *value;
}

std::vector<std::string> RequireStringArrayValues(
    const std::string& json_text,
    const std::string& key
) {
    const auto values = FindStringArrayValues(json_text, key);
    if (values.empty()) {
        throw std::runtime_error("Missing required string-array key: " + key);
    }

    return values;
}

}  // namespace marcatili::io
