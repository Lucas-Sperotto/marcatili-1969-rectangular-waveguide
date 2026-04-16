#include <exception>
#include <iostream>

#include "marcatili/io/single_guide_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/single_guide.hpp"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: solve_single_guide <input_json> [output_json]\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_json_file = argc == 3 ? argv[2] : "";

    try {
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config =
            marcatili::io::ParseSingleGuideConfig(input_text, output_json_file);
        const auto result = marcatili::SolveSingleGuide(config);

        const std::string json_report = marcatili::io::BuildSingleGuideJsonReport(
            result,
            input_file,
            output_json_file
        );

        if (output_json_file.empty()) {
            std::cout << json_report;
        } else {
            marcatili::io::WriteTextFile(output_json_file, json_report);
        }

        if (!result.config.csv_output_path.empty()) {
            const std::string csv_report =
                marcatili::io::BuildSingleGuideCsvReport(result);
            marcatili::io::WriteTextFile(result.config.csv_output_path, csv_report);
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote single-guide reports to "
                      << output_json_file;

            if (!result.config.csv_output_path.empty()) {
                std::cout << " and " << result.config.csv_output_path;
            }

            std::cout << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "solve_single_guide failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
