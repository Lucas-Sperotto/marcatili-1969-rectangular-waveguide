#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "marcatili/io/sweep_wavelength_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/physics/single_guide.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc == 2;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_sweep_wavelength <input_json>\n";
}

marcatili::SingleGuideConfig BuildPointConfig(
    const marcatili::io::SweepWavelengthConfig& config,
    const marcatili::io::SweepWavelengthModeSpec& mode,
    double wavelength
) {
    marcatili::SingleGuideConfig point_config;

    point_config.case_id = config.case_id;
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = marcatili::SingleGuideSolverModel::kExact;
    point_config.family = mode.family;
    point_config.p = mode.p;
    point_config.q = mode.q;
    point_config.wavelength = wavelength;
    point_config.a = config.a;
    point_config.b = config.b;
    point_config.n1 = config.n1;
    point_config.n2 = config.n2;
    point_config.n3 = config.n3;
    point_config.n4 = config.n4;
    point_config.n5 = config.n5;

    return point_config;
}

std::vector<marcatili::io::SweepWavelengthSample> SolveSweepWavelength(
    const marcatili::io::SweepWavelengthConfig& config
) {
    const double step =
        (config.max_wavelength - config.min_wavelength) /
        static_cast<double>(config.point_count - 1);

    std::vector<marcatili::io::SweepWavelengthSample> samples;
    samples.reserve(
        static_cast<std::size_t>(config.point_count) * config.modes.size()
    );

    for (int index = 0; index < config.point_count; ++index) {
        const double wavelength =
            config.min_wavelength + step * static_cast<double>(index);

        for (const auto& mode : config.modes) {
            const auto point_config = BuildPointConfig(config, mode, wavelength);
            const auto result = marcatili::SolveSingleGuideExact(point_config);

            samples.push_back(
                {
                    wavelength,
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
        // This executable only orchestrates a wavelength sweep. Parsing and
        // CSV formatting stay in io/, while modal physics stays in single_guide.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseSweepWavelengthConfig(input_text);
        const auto samples = SolveSweepWavelength(config);

        std::cout << marcatili::io::BuildSweepWavelengthCsvReport(samples);
    } catch (const std::exception& error) {
        std::cerr << "reproduce_sweep_wavelength failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
