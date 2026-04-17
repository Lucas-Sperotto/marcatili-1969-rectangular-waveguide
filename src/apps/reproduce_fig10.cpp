#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/fig10_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig10.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_fig10 <input_json> [output_json]\n";
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
        // - a lógica física do sweep fica em physics/
        //
        // A Fig. 10 atua como orquestrador numérico do sweep da Eq. (34),
        // enquanto legenda, estilo do artigo e renderização final ficam na
        // camada externa de plotagem.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::Figure10Config config =
            marcatili::io::ParseFigure10Config(input_text, output_json_file);

        const marcatili::Figure10Result result =
            marcatili::SolveFigure10(config);

        const std::string json_report =
            marcatili::io::BuildFigure10JsonReport(
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
                marcatili::io::BuildFigure10CsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.csv_output_path,
                csv_report
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 10 JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote Figure 10 CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig10 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
