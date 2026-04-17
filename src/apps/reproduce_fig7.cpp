#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/fig7_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig7.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_fig7 <input_json> [output_json]\n";
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
        // - a construção do nomograma fica em physics/
        //
        // A Fig. 7 produz duas camadas CSV:
        // 1. linhas do nomograma;
        // 2. interseções entre linhas modais e linhas de C.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::Figure7Config config =
            marcatili::io::ParseFigure7Config(input_text, output_json_file);

        const marcatili::Figure7Result result =
            marcatili::SolveFigure7(config);

        const std::string json_report =
            marcatili::io::BuildFigure7JsonReport(
                result,
                input_file,
                output_json_file
            );

        if (output_json_file.empty()) {
            std::cout << json_report;
        } else {
            marcatili::io::WriteTextFile(output_json_file, json_report);
        }

        if (!result.config.lines_csv_output_path.empty()) {
            const std::string lines_csv_report =
                marcatili::io::BuildFigure7LinesCsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.lines_csv_output_path,
                lines_csv_report
            );
        }

        if (!result.config.intersections_csv_output_path.empty()) {
            const std::string intersections_csv_report =
                marcatili::io::BuildFigure7IntersectionsCsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.intersections_csv_output_path,
                intersections_csv_report
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 7 JSON report to "
                      << output_json_file << "\n";

            if (!result.config.lines_csv_output_path.empty()) {
                std::cout << "Wrote Figure 7 lines CSV report to "
                          << result.config.lines_csv_output_path << "\n";
            }

            if (!result.config.intersections_csv_output_path.empty()) {
                std::cout << "Wrote Figure 7 intersections CSV report to "
                          << result.config.intersections_csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig7 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
