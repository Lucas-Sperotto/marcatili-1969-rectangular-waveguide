#pragma once

#include <optional>
#include <string>

#include <vector>

namespace marcatili::io {

// These helpers intentionally support only the small, controlled JSON schema
// used by the repository inputs. They are not meant to be general JSON parsers.
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

}  // namespace marcatili::io

