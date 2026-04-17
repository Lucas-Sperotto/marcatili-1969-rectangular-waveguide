#pragma once

#include <optional>
#include <string>

#include <vector>

namespace marcatili::io {

// These helpers intentionally support only the small, controlled JSON schema
// used by the repository inputs. They are not meant to be general JSON parsers.
// Keys may be provided as dotted paths (for example: "geometry.a").
//
// Supported value kinds are intentionally restricted:
// - strings, doubles, ints;
// - arrays of strings;
// - arrays of objects (returned as raw JSON object strings).
// This is a deliberate design tradeoff for a dependency-free, stable input layer.
std::optional<std::string> FindStringValue(
    const std::string& json_text,
    const std::string& key
);

std::optional<double> FindDoubleValue(
    const std::string& json_text,
    const std::string& key
);

std::optional<int> FindIntValue(
    const std::string& json_text,
    const std::string& key
);

std::vector<std::string> FindStringArrayValues(
    const std::string& json_text,
    const std::string& key
);

std::optional<std::string> FindRawJsonValue(
    const std::string& json_text,
    const std::string& key
);

std::vector<std::string> FindObjectArrayValues(
    const std::string& json_text,
    const std::string& key
);

std::string RequireStringValue(
    const std::string& json_text,
    const std::string& key
);

double RequireDoubleValue(
    const std::string& json_text,
    const std::string& key
);

int RequireIntValue(
    const std::string& json_text,
    const std::string& key
);

std::vector<std::string> RequireStringArrayValues(
    const std::string& json_text,
    const std::string& key
);

std::vector<std::string> RequireObjectArrayValues(
    const std::string& json_text,
    const std::string& key
);

}  // namespace marcatili::io
