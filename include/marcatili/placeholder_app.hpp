#pragma once

#include <string>

namespace marcatili {

// Lightweight descriptor used to keep each placeholder executable explicit
// about which artifact of the paper it will eventually reproduce.
struct PlaceholderAppSpec {
    std::string executable_name;
    std::string objective;
};

// Shared bootstrap runner for the initial repository stage.
// Every executable accepts an input file and can optionally write a JSON report.
int RunPlaceholderApp(const PlaceholderAppSpec& spec, int argc, char** argv);

}  // namespace marcatili

