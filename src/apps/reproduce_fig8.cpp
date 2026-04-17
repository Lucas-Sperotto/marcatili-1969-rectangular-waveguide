#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/fig8_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig8.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_fig8 <input_json> [output_json]\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (!HasValidCliArguments(argc)) {
        PrintUsage();
        return 1;
    }

    const std::string input_file = argv[1];
    const std::string output_json_file = (argc == 3) ? argv[2] : "";

    try {
        // Este executável é deliberadamente fino:
        // - leitura e escrita ficam em io/
        // - parsing do schema fica em io/
        // - a física do sweep fica em physics/
        //
        // A Fig. 8 gera um relatório JSON e um CSV com os traços numéricos
        // usados depois pelos scripts externos de comparação e plotagem.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::Figure8Config config =
            marcatili::io::ParseFigure8Config(input_text, output_json_file);

        const marcatili::Figure8Result result =
            marcatili::SolveFigure8(config);

        const std::string json_report =
            marcatili::io::BuildFigure8JsonReport(
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
                marcatili::io::BuildFigure8CsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.csv_output_path,
                csv_report
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 8 JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote Figure 8 CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig8 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
