/**
 * @file
 * @brief Executável para resolver um ponto do acoplador direcional.
 *
 * @details
 * Este programa:
 * 1. lê um arquivo JSON de entrada;
 * 2. monta a configuração do ponto do acoplador;
 * 3. resolve o modelo normalizado de acoplamento;
 * 4. gera relatórios em JSON e, opcionalmente, em CSV.
 *
 * A matemática do problema fica concentrada em `physics/`,
 * enquanto este arquivo atua apenas como ponto de entrada reprodutível.
 */

#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/coupler_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/coupler.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: solve_coupler <input_json> [output_json]\n";
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
        // - a física do acoplador fica em physics/
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::CouplerPointConfig config =
            marcatili::io::ParseCouplerPointConfig(input_text, output_json_file);

        const marcatili::CouplerPointResult result =
            marcatili::SolveCouplerPoint(config);

        const std::string json_report =
            marcatili::io::BuildCouplerPointJsonReport(
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
                marcatili::io::BuildCouplerPointCsvReport(result);

            marcatili::io::WriteTextFile(result.config.csv_output_path, csv_report);
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote coupler point JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote coupler point CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "solve_coupler failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
