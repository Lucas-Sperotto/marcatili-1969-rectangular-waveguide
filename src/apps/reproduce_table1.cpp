/**
 * @file
 * @brief Application to reproduce Table I from Marcatili (1969).
 * @details This executable reads a JSON configuration, performs a numerical
 * search for the cutoff dimensions for single-mode operation, and writes
 * a summary and detailed results to JSON and CSV files.
 */

#include <exception>
#include <iostream>

#include "marcatili/io/table1_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/table1.hpp"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: reproduce_table1 <input_json> [output_json]\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_json_file = argc == 3 ? argv[2] : "";

    try {
        // Table I is not a single direct formula in the repository; it is a search
        // over mode cutoffs whose results are stored both as row summaries and as
        // per-mode diagnostic details.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseTable1Config(input_text, output_json_file);
        const auto result = marcatili::SolveTable1(config);

        const std::string json_report =
            marcatili::io::BuildTable1JsonReport(result, input_file, output_json_file);

        if (output_json_file.empty()) {
            std::cout << json_report;
        } else {
            marcatili::io::WriteTextFile(output_json_file, json_report);
        }

        if (!result.config.summary_csv_output_path.empty()) {
            marcatili::io::WriteTextFile(
                result.config.summary_csv_output_path, marcatili::io::BuildTable1SummaryCsvReport(result));
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Table I reports to " << output_json_file << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_table1 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
