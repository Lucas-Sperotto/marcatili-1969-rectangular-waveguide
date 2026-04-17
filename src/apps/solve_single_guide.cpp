/**
 * @file
 * @brief Executável para resolver um ponto de um guia dielétrico retangular.
 *
 * @details
 * Este programa:
 * 1. lê um arquivo JSON de entrada;
 * 2. monta a configuração do guia único;
 * 3. resolve o problema com o solver selecionado (`closed_form` ou `exact`);
 * 4. gera relatórios em JSON e, opcionalmente, em CSV.
 *
 * A física do problema fica concentrada em `physics/`,
 * enquanto este arquivo atua apenas como ponto de entrada reprodutível.
 */

#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/single_guide_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/single_guide.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: solve_single_guide <input_json> [output_json]\n";
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
        // - a física do guia fica em physics/
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::SingleGuideConfig config =
            marcatili::io::ParseSingleGuideConfig(input_text, output_json_file);

        const marcatili::SingleGuideResult result =
            marcatili::SolveSingleGuide(config);

        const std::string json_report =
            marcatili::io::BuildSingleGuideJsonReport(
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
            std::cout << "Wrote single guide JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote single guide CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "solve_single_guide failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
