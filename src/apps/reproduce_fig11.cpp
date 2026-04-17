/**
 * @file
 * @brief Executável para reproduzir a Figura 11 de Marcatili (1969).
 *
 * @details
 * Este programa:
 * 1. lê um arquivo JSON de entrada;
 * 2. monta a configuração da Fig. 11;
 * 3. executa o sweep do acoplador direcional;
 * 4. gera um relatório JSON e um CSV com as curvas calculadas.
 *
 * A Fig. 11 reutiliza o mesmo núcleo normalizado do acoplador usado na Fig. 10,
 * mas muda a equação transversal e varre os casos de razão de índices da legenda.
 */

#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/fig11_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/fig11.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_fig11 <input_json> [output_json]\n";
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

        const marcatili::Figure11Config config =
            marcatili::io::ParseFigure11Config(input_text, output_json_file);

        const marcatili::Figure11Result result =
            marcatili::SolveFigure11(config);

        const std::string json_report =
            marcatili::io::BuildFigure11JsonReport(
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
                marcatili::io::BuildFigure11CsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.csv_output_path,
                csv_report
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Figure 11 JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote Figure 11 CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_fig11 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
