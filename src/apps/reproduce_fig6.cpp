/**
 * @file
 * @brief Executável para reproduzir a Figura 6 de Marcatili (1969).
 *
 * @details
 * Este programa:
 * 1. lê um arquivo JSON de entrada;
 * 2. monta a configuração de um painel da Fig. 6;
 * 3. executa o sweep numérico correspondente;
 * 4. gera um relatório JSON e um CSV com as amostras das curvas.
 *
 * A física e o sweep ficam concentrados em `physics/`,
 * enquanto este arquivo atua como ponto de entrada reprodutível.
 */

#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/fig6_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig6.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_fig6 <input_json> [output_json]\n";
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
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::Figure6Config config =
            marcatili::io::ParseFigure6Config(input_text, output_json_file);

        const marcatili::Figure6Result result =
            marcatili::SolveFigure6(config);

        const std::string json_report =
            marcatili::io::BuildFigure6JsonReport(
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
                marcatili::io::BuildFigure6CsvReport(result);

            marcatili::io::WriteTextFile(result.config.csv_output_path, csv_report);
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 6 JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote Figure 6 CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig6 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
