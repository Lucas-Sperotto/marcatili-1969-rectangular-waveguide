/**
 * @file
 * @brief Executável para reproduzir a Tabela I de Marcatili (1969).
 *
 * @details
 * Este programa:
 * 1. lê um arquivo JSON de entrada;
 * 2. monta a configuração da Tabela I;
 * 3. executa a busca numérica dos cutoffs modais;
 * 4. gera um relatório JSON, um CSV-resumo e um CSV detalhado.
 *
 * Diferentemente das figuras, a Tabela I não é uma simples varredura direta:
 * ela depende de uma busca sobre os modos superiores que limitam a operação
 * monomodo para cada linha considerada.
 */

#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/table1_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/table1.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_table1 <input_json> [output_json]\n";
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
        // - a busca física dos cutoffs fica em physics/
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::Table1Config config =
            marcatili::io::ParseTable1Config(input_text, output_json_file);

        const marcatili::Table1Result result =
            marcatili::SolveTable1(config);

        const std::string json_report =
            marcatili::io::BuildTable1JsonReport(
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
            const std::string summary_csv_report =
                marcatili::io::BuildTable1SummaryCsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.summary_csv_output_path,
                summary_csv_report
            );
        }

        if (!result.config.details_csv_output_path.empty()) {
            const std::string details_csv_report =
                marcatili::io::BuildTable1DetailsCsvReport(result);

            marcatili::io::WriteTextFile(
                result.config.details_csv_output_path,
                details_csv_report
            );
        }

        if (!output_json_file.empty()) {
            std::cout << "Wrote Table I JSON report to "
                      << output_json_file << "\n";

            if (!result.config.summary_csv_output_path.empty()) {
                std::cout << "Wrote Table I summary CSV report to "
                          << result.config.summary_csv_output_path << "\n";
            }

            if (!result.config.details_csv_output_path.empty()) {
                std::cout << "Wrote Table I details CSV report to "
                          << result.config.details_csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "reproduce_table1 failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
