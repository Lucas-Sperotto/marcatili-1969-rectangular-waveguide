/**
 * @file
 * @brief Application to solve a single point for a directional coupler.
 * @details This executable reads a JSON configuration, solves for the
 * normalized coupling coefficient based on Eq. (34) of Marcatili's paper,
 * and writes the results to JSON and CSV files.
 */

#include <exception>
#include <iostream>

#include "marcatili/io/coupler_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/coupler.hpp"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: solve_coupler <input_json> [output_json]\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_json_file = argc == 3 ? argv[2] : "";

    try {
        // This wrapper intentionally stays thin: the executable is a reproducible
        // entry point, while the normalized coupler mathematics lives in physics/.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseCouplerPointConfig(input_text, output_json_file);
        const auto result = marcatili::SolveCouplerPoint(config);

        const std::string json_report =
            marcatili::io::BuildCouplerPointJsonReport(result, input_file, output_json_file);

        if (output_json_file.empty()) {
            std::cout << json_report;
        } else {
            marcatili::io::WriteTextFile(output_json_file, json_report);
        }

        if (!result.config.csv_output_path.empty()) {
            marcatili::io::WriteTextFile(
                result.config.csv_output_path, marcatili::io::BuildCouplerPointCsvReport(result));
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote coupler point reports to " << output_json_file << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "solve_coupler failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
