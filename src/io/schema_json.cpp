#include "marcatili/io/schema_json.hpp"

#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace marcatili::io {
namespace {

bool IsWhitespace(char character) {
    return std::isspace(static_cast<unsigned char>(character)) != 0;
}

std::size_t SkipWhitespace(std::string_view text, std::size_t position) {
    std::size_t current = position;
    while (current < text.size() && IsWhitespace(text[current])) {
        ++current;
    }

    return current;
}

std::string_view TrimWhitespace(std::string_view text) {
    const std::size_t begin = SkipWhitespace(text, 0);
    if (begin >= text.size()) {
        return {};
    }

    std::size_t end = text.size();
    while (end > begin && IsWhitespace(text[end - 1])) {
        --end;
    }

    return text.substr(begin, end - begin);
}

std::optional<std::size_t> FindMatchingDelimiter(
    std::string_view text,
    std::size_t open_position,
    char open_delimiter,
    char close_delimiter
) {
    if (open_position >= text.size() || text[open_position] != open_delimiter) {
        return std::nullopt;
    }

    int depth = 0;
    bool in_string = false;
    bool escaping = false;

    for (std::size_t position = open_position; position < text.size(); ++position) {
        const char character = text[position];

        if (in_string) {
            if (escaping) {
                escaping = false;
            } else if (character == '\\') {
                escaping = true;
            } else if (character == '"') {
                in_string = false;
            }

            continue;
        }

        if (character == '"') {
            in_string = true;
            continue;
        }

        if (character == open_delimiter) {
            ++depth;
            continue;
        }

        if (character == close_delimiter) {
            --depth;
            if (depth == 0) {
                return position;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::pair<std::size_t, std::string>> ParseJsonStringToken(
    std::string_view text,
    std::size_t quote_position
) {
    if (quote_position >= text.size() || text[quote_position] != '"') {
        return std::nullopt;
    }

    std::string decoded;
    bool escaping = false;

    for (std::size_t position = quote_position + 1; position < text.size(); ++position) {
        const char character = text[position];

        if (escaping) {
            switch (character) {
                case '"':
                case '\\':
                case '/':
                    decoded.push_back(character);
                    break;
                case 'b':
                    decoded.push_back('\b');
                    break;
                case 'f':
                    decoded.push_back('\f');
                    break;
                case 'n':
                    decoded.push_back('\n');
                    break;
                case 'r':
                    decoded.push_back('\r');
                    break;
                case 't':
                    decoded.push_back('\t');
                    break;
                case 'u':
                    // Os inputs do repositório são ASCII na prática.
                    // Mantemos suporte mínimo e estável sem depender de biblioteca externa.
                    decoded.push_back('?');
                    if (position + 4 < text.size()) {
                        position += 4;
                    }
                    break;
                default:
                    decoded.push_back(character);
                    break;
            }

            escaping = false;
            continue;
        }

        if (character == '\\') {
            escaping = true;
            continue;
        }

        if (character == '"') {
            return std::make_pair(position + 1, decoded);
        }

        decoded.push_back(character);
    }

    return std::nullopt;
}

std::optional<std::size_t> ParseJsonValueEnd(
    std::string_view object_body,
    std::size_t position
) {
    if (position >= object_body.size()) {
        return std::nullopt;
    }

    const char character = object_body[position];

    if (character == '"') {
        const auto token = ParseJsonStringToken(object_body, position);
        if (!token.has_value()) {
            return std::nullopt;
        }

        return token->first;
    }

    if (character == '{') {
        const auto closing = FindMatchingDelimiter(object_body, position, '{', '}');
        if (!closing.has_value()) {
            return std::nullopt;
        }

        return *closing + 1;
    }

    if (character == '[') {
        const auto closing = FindMatchingDelimiter(object_body, position, '[', ']');
        if (!closing.has_value()) {
            return std::nullopt;
        }

        return *closing + 1;
    }

    std::size_t end = position;
    while (end < object_body.size() && object_body[end] != ',') {
        ++end;
    }

    return end;
}

std::optional<std::string_view> RootObjectBody(std::string_view json_text) {
    const std::string_view trimmed = TrimWhitespace(json_text);

    if (trimmed.empty() || trimmed.front() != '{') {
        return std::nullopt;
    }

    const auto closing = FindMatchingDelimiter(trimmed, 0, '{', '}');
    if (!closing.has_value()) {
        return std::nullopt;
    }

    if (SkipWhitespace(trimmed, *closing + 1) != trimmed.size()) {
        return std::nullopt;
    }

    return trimmed.substr(1, *closing - 1);
}

std::optional<std::string_view> ObjectBodyFromRawValue(std::string_view raw_value) {
    const std::string_view trimmed = TrimWhitespace(raw_value);

    if (trimmed.empty() || trimmed.front() != '{') {
        return std::nullopt;
    }

    const auto closing = FindMatchingDelimiter(trimmed, 0, '{', '}');
    if (!closing.has_value()) {
        return std::nullopt;
    }

    if (SkipWhitespace(trimmed, *closing + 1) != trimmed.size()) {
        return std::nullopt;
    }

    return trimmed.substr(1, *closing - 1);
}

std::optional<std::string_view> FindTopLevelRawValueInObject(
    std::string_view object_body,
    std::string_view key
) {
    std::size_t position = 0;

    while (true) {
        position = SkipWhitespace(object_body, position);
        if (position >= object_body.size()) {
            return std::nullopt;
        }

        if (object_body[position] == ',') {
            ++position;
            continue;
        }

        if (object_body[position] != '"') {
            return std::nullopt;
        }

        const auto key_token = ParseJsonStringToken(object_body, position);
        if (!key_token.has_value()) {
            return std::nullopt;
        }

        position = SkipWhitespace(object_body, key_token->first);
        if (position >= object_body.size() || object_body[position] != ':') {
            return std::nullopt;
        }

        position = SkipWhitespace(object_body, position + 1);
        if (position >= object_body.size()) {
            return std::nullopt;
        }

        const std::size_t value_begin = position;
        const auto value_end = ParseJsonValueEnd(object_body, position);
        if (!value_end.has_value()) {
            return std::nullopt;
        }

        if (key_token->second == key) {
            return TrimWhitespace(object_body.substr(value_begin, *value_end - value_begin));
        }

        position = SkipWhitespace(object_body, *value_end);
        if (position < object_body.size() && object_body[position] == ',') {
            ++position;
        }
    }
}

std::vector<std::string> SplitPath(const std::string& key_path) {
    std::vector<std::string> segments;
    std::size_t begin = 0;

    while (begin <= key_path.size()) {
        const std::size_t end = key_path.find('.', begin);
        const std::size_t count =
            (end == std::string::npos) ? (key_path.size() - begin) : (end - begin);

        if (count == 0) {
            return {};
        }

        segments.push_back(key_path.substr(begin, count));
        if (end == std::string::npos) {
            break;
        }

        begin = end + 1;
    }

    return segments;
}

std::optional<std::string_view> FindRawValueByPath(
    const std::string& json_text,
    const std::string& key_path
) {
    const auto segments = SplitPath(key_path);
    if (segments.empty()) {
        return std::nullopt;
    }

    auto current_object = RootObjectBody(json_text);
    if (!current_object.has_value()) {
        return std::nullopt;
    }

    for (std::size_t index = 0; index < segments.size(); ++index) {
        const auto raw_value =
            FindTopLevelRawValueInObject(*current_object, segments[index]);
        if (!raw_value.has_value()) {
            return std::nullopt;
        }

        if (index + 1 == segments.size()) {
            return raw_value;
        }

        current_object = ObjectBodyFromRawValue(*raw_value);
        if (!current_object.has_value()) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::optional<std::string> ParseRawJsonStringValue(std::string_view raw_value) {
    const std::string_view trimmed = TrimWhitespace(raw_value);
    if (trimmed.empty() || trimmed.front() != '"') {
        return std::nullopt;
    }

    const auto token = ParseJsonStringToken(trimmed, 0);
    if (!token.has_value()) {
        return std::nullopt;
    }

    if (SkipWhitespace(trimmed, token->first) != trimmed.size()) {
        return std::nullopt;
    }

    return token->second;
}

std::optional<double> ParseRawJsonDoubleValue(std::string_view raw_value) {
    const std::string text(TrimWhitespace(raw_value));
    if (text.empty()) {
        return std::nullopt;
    }

    std::size_t parsed_characters = 0;
    try {
        const double value = std::stod(text, &parsed_characters);
        if (parsed_characters != text.size()) {
            return std::nullopt;
        }

        return value;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int> ParseRawJsonIntValue(std::string_view raw_value) {
    const std::string text(TrimWhitespace(raw_value));
    if (text.empty()) {
        return std::nullopt;
    }

    std::size_t parsed_characters = 0;
    try {
        const int value = std::stoi(text, &parsed_characters);
        if (parsed_characters != text.size()) {
            return std::nullopt;
        }

        return value;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::vector<std::string>> ParseRawJsonStringArray(std::string_view raw_value) {
    const std::string_view trimmed = TrimWhitespace(raw_value);
    if (trimmed.empty() || trimmed.front() != '[') {
        return std::nullopt;
    }

    const auto closing = FindMatchingDelimiter(trimmed, 0, '[', ']');
    if (!closing.has_value()) {
        return std::nullopt;
    }

    if (SkipWhitespace(trimmed, *closing + 1) != trimmed.size()) {
        return std::nullopt;
    }

    const std::string_view array_body = trimmed.substr(1, *closing - 1);
    std::vector<std::string> values;
    std::size_t position = 0;

    while (true) {
        position = SkipWhitespace(array_body, position);
        if (position >= array_body.size()) {
            break;
        }

        if (array_body[position] == ',') {
            ++position;
            continue;
        }

        if (array_body[position] != '"') {
            return std::nullopt;
        }

        const auto token = ParseJsonStringToken(array_body, position);
        if (!token.has_value()) {
            return std::nullopt;
        }

        values.push_back(token->second);
        position = SkipWhitespace(array_body, token->first);

        if (position < array_body.size() && array_body[position] == ',') {
            ++position;
        }
    }

    return values;
}

std::optional<std::vector<std::string>> ParseRawJsonObjectArray(std::string_view raw_value) {
    const std::string_view trimmed = TrimWhitespace(raw_value);
    if (trimmed.empty() || trimmed.front() != '[') {
        return std::nullopt;
    }

    const auto closing = FindMatchingDelimiter(trimmed, 0, '[', ']');
    if (!closing.has_value()) {
        return std::nullopt;
    }

    if (SkipWhitespace(trimmed, *closing + 1) != trimmed.size()) {
        return std::nullopt;
    }

    const std::string_view array_body = trimmed.substr(1, *closing - 1);
    std::vector<std::string> values;
    std::size_t position = 0;

    while (true) {
        position = SkipWhitespace(array_body, position);
        if (position >= array_body.size()) {
            break;
        }

        if (array_body[position] == ',') {
            ++position;
            continue;
        }

        if (array_body[position] != '{') {
            return std::nullopt;
        }

        const auto object_end =
            FindMatchingDelimiter(array_body, position, '{', '}');
        if (!object_end.has_value()) {
            return std::nullopt;
        }

        values.emplace_back(
            TrimWhitespace(array_body.substr(position, *object_end - position + 1))
        );

        position = SkipWhitespace(array_body, *object_end + 1);
        if (position < array_body.size() && array_body[position] == ',') {
            ++position;
        }
    }

    return values;
}

std::string RequireRawValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        throw std::runtime_error("Missing required key: " + key);
    }

    return std::string(*raw_value);
}

}  // namespace

std::optional<std::string> FindStringValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        return std::nullopt;
    }

    return ParseRawJsonStringValue(*raw_value);
}

std::optional<double> FindDoubleValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        return std::nullopt;
    }

    return ParseRawJsonDoubleValue(*raw_value);
}

std::optional<int> FindIntValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        return std::nullopt;
    }

    return ParseRawJsonIntValue(*raw_value);
}

std::vector<std::string> FindStringArrayValues(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        return {};
    }

    const auto parsed = ParseRawJsonStringArray(*raw_value);
    return parsed.has_value() ? *parsed : std::vector<std::string>{};
}

std::optional<std::string> FindRawJsonValue(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        return std::nullopt;
    }

    return std::string(*raw_value);
}

std::vector<std::string> FindObjectArrayValues(
    const std::string& json_text,
    const std::string& key
) {
    const auto raw_value = FindRawValueByPath(json_text, key);
    if (!raw_value.has_value()) {
        return {};
    }

    const auto parsed = ParseRawJsonObjectArray(*raw_value);
    return parsed.has_value() ? *parsed : std::vector<std::string>{};
}

std::string RequireStringValue(
    const std::string& json_text,
    const std::string& key
) {
    const std::string raw_value = RequireRawValue(json_text, key);
    const auto value = ParseRawJsonStringValue(raw_value);

    if (!value.has_value()) {
        throw std::runtime_error("Invalid required string key: " + key);
    }

    return *value;
}

double RequireDoubleValue(
    const std::string& json_text,
    const std::string& key
) {
    const std::string raw_value = RequireRawValue(json_text, key);
    const auto value = ParseRawJsonDoubleValue(raw_value);

    if (!value.has_value()) {
        throw std::runtime_error("Invalid required numeric key: " + key);
    }

    return *value;
}

int RequireIntValue(
    const std::string& json_text,
    const std::string& key
) {
    const std::string raw_value = RequireRawValue(json_text, key);
    const auto value = ParseRawJsonIntValue(raw_value);

    if (!value.has_value()) {
        throw std::runtime_error("Invalid required integer key: " + key);
    }

    return *value;
}

std::vector<std::string> RequireStringArrayValues(
    const std::string& json_text,
    const std::string& key
) {
    const std::string raw_value = RequireRawValue(json_text, key);
    const auto values = ParseRawJsonStringArray(raw_value);

    if (!values.has_value()) {
        throw std::runtime_error("Invalid required string-array key: " + key);
    }

    return *values;
}

std::vector<std::string> RequireObjectArrayValues(
    const std::string& json_text,
    const std::string& key
) {
    const std::string raw_value = RequireRawValue(json_text, key);
    const auto values = ParseRawJsonObjectArray(raw_value);

    if (!values.has_value()) {
        throw std::runtime_error("Invalid required object-array key: " + key);
    }

    return *values;
}

}  // namespace marcatili::io
