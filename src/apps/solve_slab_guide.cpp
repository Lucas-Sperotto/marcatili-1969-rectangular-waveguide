/**
 * @file
 * @brief Executável para resolver um ponto no limite de lâmina do guia.
 *
 * @details
 * Este programa reutiliza o mesmo schema de entrada do `solve_single_guide`,
 * mas delega a física para `SolveSlabGuide`. A saída permanece compatível
 * com o relatório CSV/JSON do guia único para facilitar comparações diretas.
 */

#include <exception>
#include <iostream>
#include <string>

#include "marcatili/io/single_guide_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/slab_guide.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc >= 2 && argc <= 3;
}

void PrintUsage() {
    std::cerr << "Usage: solve_slab_guide <input_json> [output_json]\n";
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
        const std::string input_text = marcatili::io::ReadTextFile(input_file);

        const marcatili::SingleGuideConfig config =
            marcatili::io::ParseSingleGuideConfig(input_text, output_json_file);

        const marcatili::SingleGuideResult result =
            marcatili::SolveSlabGuide(config);

        const std::string json_report =
            marcatili::io::BuildSingleGuideJsonReport(
                result,
                input_file,
                output_json_file,
                "solve_slab_guide"
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
            std::cout << "Wrote slab guide JSON report to "
                      << output_json_file << "\n";

            if (!result.config.csv_output_path.empty()) {
                std::cout << "Wrote slab guide CSV report to "
                          << result.config.csv_output_path << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "solve_slab_guide failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
