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
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseTable1Config(input_text, output_json_file);
        const auto result = marcatili::SolveTable1(config);

        const std::string json_report = marcatili::io::BuildTable1JsonReport(
            result,
            input_file,
            output_json_file
        );

        if (output_json_file.empty()) {
            std::cout << json_report;
        } else {
            marcatili::io::WriteTextFile(output_json_file, json_report);
        }

        if (!result.config.summary_csv_output_path.empty()) {
            marcatili::io::WriteTextFile(
                result.config.summary_csv_output_path,
                marcatili::io::BuildTable1SummaryCsvReport(result)
            );
        }

        if (!result.config.details_csv_output_path.empty()) {
            marcatili::io::WriteTextFile(
                result.config.details_csv_output_path,
                marcatili::io::BuildTable1DetailsCsvReport(result)
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Table I reports to " << output_json_file;

            if (!result.config.summary_csv_output_path.empty()) {
                std::cout << ", " << result.config.summary_csv_output_path;
            }

            if (!result.config.details_csv_output_path.empty()) {
                std::cout << " and " << result.config.details_csv_output_path;
            }

            std::cout << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_table1 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
