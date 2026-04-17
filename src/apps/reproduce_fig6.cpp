/**
 * @file
 * @brief Application to reproduce Figure 6 from Marcatili (1969).
 * @details This executable reads a JSON configuration for a specific panel of
 * Figure 6, performs a parameter sweep by calling the core physics solvers,
 * and writes the resulting dispersion curves to JSON and CSV files.
 */

#include <exception>
#include <iostream>

#include "marcatili/io/fig6_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig6.hpp"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: reproduce_fig6 <input_json> [output_json]\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_json_file = argc == 3 ? argv[2] : "";

    try {
        // The figure executables follow the same pattern: read JSON, run a sweep,
        // emit a rich JSON summary plus CSV samples for external Python plotting.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseFigure6Config(input_text, output_json_file);
        const auto result = marcatili::SolveFigure6(config);

        const std::string json_report =
            marcatili::io::BuildFigure6JsonReport(result, input_file, output_json_file);

        if (output_json_file.empty()) {
            std::cout << json_report;
        } else {
            marcatili::io::WriteTextFile(output_json_file, json_report);
        }

        if (!result.config.csv_output_path.empty()) {
            marcatili::io::WriteTextFile(
                result.config.csv_output_path, marcatili::io::BuildFigure6CsvReport(result));
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 6 reports to " << output_json_file << " and "
                      << result.config.csv_output_path << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig6 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
