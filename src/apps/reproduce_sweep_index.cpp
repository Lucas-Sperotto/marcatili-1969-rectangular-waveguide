#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "marcatili/io/sweep_index_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/single_guide.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc == 2;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_sweep_index <input_json>\n";
}

marcatili::SingleGuideConfig BuildPointConfig(
    const marcatili::io::SweepIndexConfig& config,
    const marcatili::io::SweepIndexModeSpec& mode,
    double n4
) {
    marcatili::SingleGuideConfig point_config;

    point_config.case_id = config.case_id;
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = marcatili::SingleGuideSolverModel::kExact;
    point_config.family = mode.family;
    point_config.p = mode.p;
    point_config.q = mode.q;
    point_config.wavelength = config.base_case.wavelength;
    point_config.a = config.base_case.a;
    point_config.b = config.base_case.b;
    point_config.n1 = config.base_case.n1;
    point_config.n2 = config.base_case.n2;
    point_config.n3 = config.base_case.n3;
    point_config.n4 = n4;
    point_config.n5 = config.base_case.n5;

    return point_config;
}

std::vector<marcatili::io::SweepIndexSample> SolveSweepIndex(
    const marcatili::io::SweepIndexConfig& config
) {
    std::vector<marcatili::io::SweepIndexSample> samples;
    samples.reserve(config.sweep_values.size() * config.modes.size());

    for (const double n4 : config.sweep_values) {
        for (const auto& mode : config.modes) {
            const auto point_config = BuildPointConfig(config, mode, n4);
            const auto result = marcatili::SolveSingleGuideExact(point_config);

            samples.push_back(
                {
                    n4,
                    mode.family,
                    mode.p,
                    mode.q,
                    result.kz,
                    result.ky,
                    result.kx
                }
            );
        }
    }

    return samples;
}

}  // namespace

int main(int argc, char** argv) {
    if (!HasValidCliArguments(argc)) {
        PrintUsage();
        return 1;
    }

    const std::string input_file = argv[1];

    try {
        // This executable only orchestrates an index sweep. Parsing and CSV
        // formatting stay in io/, while modal physics stays in single_guide.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseSweepIndexConfig(input_text);
        const auto samples = SolveSweepIndex(config);

        std::cout << marcatili::io::BuildSweepIndexCsvReport(samples);
    } catch (const std::exception& error) {
        std::cerr << "reproduce_sweep_index failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
