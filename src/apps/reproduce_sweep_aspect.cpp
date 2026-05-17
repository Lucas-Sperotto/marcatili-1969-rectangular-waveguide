#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "marcatili/io/sweep_aspect_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/math/waveguide_math.hpp"
#include "marcatili/physics/single_guide.hpp"

namespace {

bool HasValidCliArguments(int argc) {
    return argc == 2;
}

void PrintUsage() {
    std::cerr << "Usage: reproduce_sweep_aspect <input_json>\n";
}

marcatili::SingleGuideConfig BuildPointConfig(
    const marcatili::io::SweepAspectConfig& config,
    const marcatili::io::SweepAspectModeSpec& mode,
    double a_over_b,
    double b
) {
    marcatili::SingleGuideConfig point_config;

    point_config.case_id = config.case_id;
    point_config.article_target = config.article_target;
    point_config.csv_output_path = "";
    point_config.solver_model = marcatili::SingleGuideSolverModel::kExact;
    point_config.family = mode.family;
    point_config.p = mode.p;
    point_config.q = mode.q;
    point_config.wavelength = config.wavelength;
    point_config.a = a_over_b * b;
    point_config.b = b;
    point_config.n1 = config.n1;
    point_config.n2 = config.n2;
    point_config.n3 = config.n3;
    point_config.n4 = config.n4;
    point_config.n5 = config.n5;

    return point_config;
}

std::vector<marcatili::io::SweepAspectSample> SolveSweepAspect(
    const marcatili::io::SweepAspectConfig& config
) {
    const double A4 =
        marcatili::math::ComputeA(config.wavelength, config.n1, config.n4);
    const double b = config.b_over_A4 * A4;
    const double step =
        (config.a_over_b_max - config.a_over_b_min) /
        static_cast<double>(config.point_count - 1);

    std::vector<marcatili::io::SweepAspectSample> samples;
    samples.reserve(
        static_cast<std::size_t>(config.point_count) * config.modes.size()
    );

    for (int index = 0; index < config.point_count; ++index) {
        const double a_over_b =
            config.a_over_b_min + step * static_cast<double>(index);

        for (const auto& mode : config.modes) {
            const auto point_config =
                BuildPointConfig(config, mode, a_over_b, b);
            const auto result =
                marcatili::SolveSingleGuideExact(point_config);

            samples.push_back(
                {
                    a_over_b,
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
        // This executable only orchestrates a parameter sweep. Parsing and CSV
        // formatting stay in io/, while modal physics stays in single_guide.
        const std::string input_text = marcatili::io::ReadTextFile(input_file);
        const auto config = marcatili::io::ParseSweepAspectConfig(input_text);
        const auto samples = SolveSweepAspect(config);

        std::cout << marcatili::io::BuildSweepAspectCsvReport(samples);
    } catch (const std::exception& error) {
        std::cerr << "reproduce_sweep_aspect failed: " << error.what() << "\n";
        return 2;
    }

    return 0;
}
