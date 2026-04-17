#include <exception>
#include <iostream>

#include "marcatili/io/fig11_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig11.hpp"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: reproduce_fig11 <input_json> [output_json]\n";
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_json_file = argc == 3 ? argv[2] : "";

    try {
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseFigure11Config(input_text, output_json_file);
        const auto result = marcatili::SolveFigure11(config);

        const std::string json_report = marcatili::io::BuildFigure11JsonReport(
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
            marcatili::io::WriteTextFile(
                result.config.csv_output_path,
                marcatili::io::BuildFigure11CsvReport(result)
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 11 reports to " << output_json_file;

            if (!result.config.csv_output_path.empty()) {
                std::cout << " and " << result.config.csv_output_path;
            }

            std::cout << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig11 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
