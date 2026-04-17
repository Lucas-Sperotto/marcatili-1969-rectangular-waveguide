#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

#include "marcatili/io/fig10_io.hpp"
#include "marcatili/io/fig11_io.hpp"
#include "marcatili/io/fig6_io.hpp"
#include "marcatili/io/fig7_io.hpp"
#include "marcatili/io/fig8_io.hpp"
#include "marcatili/io/single_guide_io.hpp"
#include "marcatili/io/table1_io.hpp"
#include "marcatili/io/text_io.hpp"
#include "marcatili/math/waveguide_math.hpp"
#include "marcatili/physics/coupler.hpp"
#include "marcatili/physics/fig10.hpp"
#include "marcatili/physics/fig11.hpp"
#include "marcatili/physics/fig6.hpp"
#include "marcatili/physics/fig7.hpp"
#include "marcatili/physics/fig8.hpp"
#include "marcatili/physics/single_guide.hpp"
#include "marcatili/physics/table1.hpp"

namespace {

using marcatili::CouplerPointConfig;
using marcatili::CouplerTransverseEquation;
using marcatili::Figure10Result;
using marcatili::Figure11Result;
using marcatili::Figure6Result;
using marcatili::Figure7Result;
using marcatili::Figure8Result;
using marcatili::SingleGuideConfig;
using marcatili::SingleGuideFamily;
using marcatili::SingleGuideSolverModel;
using marcatili::Table1Result;

struct TestContext {
    int failures = 0;

    void Expect(bool condition, const std::string& message) {
        if (!condition) {
            ++failures;
            std::cerr << "FAIL: " << message << "\n";
        }
    }

    void ExpectNear(
        double actual,
        double expected,
        double tolerance,
        const std::string& message
    ) {
        if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance) {
            ++failures;
            std::cerr << "FAIL: " << message
                      << " (expected " << expected
                      << ", got " << actual << ")\n";
        }
    }
};

std::string SourcePath(const std::string& relative_path) {
    return std::string(MARCATILI_PROJECT_SOURCE_DIR) + "/" + relative_path;
}

const marcatili::Figure6Sample* FindFigure6Sample(
    const Figure6Result& result,
    const std::string& curve_id,
    SingleGuideSolverModel solver_model,
    int sample_index
) {
    for (const auto& sample : result.samples) {
        if (sample.curve_id == curve_id &&
            sample.point.config.solver_model == solver_model &&
            sample.sample_index == sample_index) {
            return &sample;
        }
    }

    return nullptr;
}

const marcatili::Figure7Intersection* FindFigure7Intersection(
    const Figure7Result& result,
    const std::string& mode_line_id,
    const std::string& c_line_id
) {
    for (const auto& intersection : result.intersections) {
        if (intersection.mode_line_id == mode_line_id &&
            intersection.c_line_id == c_line_id) {
            return &intersection;
        }
    }

    return nullptr;
}

const marcatili::Figure8Sample* FindFigure8Sample(
    const Figure8Result& result,
    const std::string& curve_id,
    SingleGuideSolverModel solver_model,
    int sample_index
) {
    for (const auto& sample : result.samples) {
        if (sample.curve_id == curve_id &&
            sample.point.config.solver_model == solver_model &&
            sample.sample_index == sample_index) {
            return &sample;
        }
    }

    return nullptr;
}

const marcatili::Figure10Sample* FindFigure10Sample(
    const Figure10Result& result,
    const std::string& curve_id,
    SingleGuideSolverModel solver_model,
    int sample_index
) {
    for (const auto& sample : result.samples) {
        if (sample.curve_id == curve_id &&
            sample.point.config.solver_model == solver_model &&
            sample.sample_index == sample_index) {
            return &sample;
        }
    }

    return nullptr;
}

const marcatili::Figure11Sample* FindFigure11Sample(
    const Figure11Result& result,
    const std::string& ratio_id,
    const std::string& curve_id,
    SingleGuideSolverModel solver_model,
    int sample_index
) {
    for (const auto& sample : result.samples) {
        if (sample.ratio_id == ratio_id &&
            sample.curve_id == curve_id &&
            sample.point.config.solver_model == solver_model &&
            sample.sample_index == sample_index) {
            return &sample;
        }
    }

    return nullptr;
}

const marcatili::Table1RowSummary* FindTable1RowSummary(
    const Table1Result& result,
    const std::string& row_id,
    SingleGuideSolverModel solver_model
) {
    for (const auto& row : result.row_summaries) {
        if (row.row_id == row_id && row.solver_model == solver_model) {
            return &row;
        }
    }

    return nullptr;
}

void TestGuidedCriterionUsesCriticalExternalMedium(TestContext& tests) {
    SingleGuideConfig config;
    config.case_id = "asymmetric_guidance_check";
    config.solver_model = SingleGuideSolverModel::kClosedForm;
    config.family = SingleGuideFamily::kEy;
    config.p = 1;
    config.q = 1;
    config.wavelength = 1.0e-6;
    config.a = 1.0e-6;
    config.b = 0.5e-6;
    config.n1 = 1.5;
    config.n2 = 1.0;
    config.n3 = 1.25;
    config.n4 = 1.0;
    config.n5 = 1.0;

    const auto result = marcatili::SolveSingleGuide(config);

    tests.Expect(!result.guided, "asymmetric guide should not be marked as guided");
    tests.Expect(result.status == "below_cutoff", "asymmetric guide should report below_cutoff");
    tests.Expect(result.domain_valid, "asymmetric guide remains inside the model domain");
    tests.ExpectNear(
        result.critical_external_index,
        1.25,
        1e-12,
        "critical external index should match the largest cladding index"
    );
    tests.ExpectNear(
        result.critical_external_wave_number,
        result.k3,
        1e-9,
        "critical external wave number should track the limiting external medium"
    );
}

void TestExactBelowCutoffClassification(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/fig6/SG-006h.json"));
    const auto config = marcatili::io::ParseFigure6Config(input_text, "");
    const auto result = marcatili::SolveFigure6(config);
    const auto* sample =
        FindFigure6Sample(result, "E_y_1_1", SingleGuideSolverModel::kExact, 0);

    tests.Expect(sample != nullptr, "figure 6 exact sample should be available");
    if (sample == nullptr) {
        return;
    }

    tests.Expect(
        sample->point.status == "below_cutoff",
        "exact figure 6 sample below cutoff should not be reported as outside_exact_domain"
    );
    tests.Expect(sample->point.domain_valid, "below-cutoff exact sample should keep domain_valid");
    tests.Expect(!sample->point.guided, "below-cutoff exact sample should not be guided");
    tests.Expect(
        std::isnan(sample->point.kz),
        "below-cutoff exact sample should leave kz unavailable"
    );
}

void TestCouplerNoRootIsPhysicalStatus(TestContext& tests) {
    CouplerPointConfig config;
    config.case_id = "coupler_no_root";
    config.solver_model = SingleGuideSolverModel::kExact;
    config.transverse_equation = CouplerTransverseEquation::kEq6;
    config.p = 2;
    config.a_over_A5 = 0.1;
    config.c_over_a = 0.5;

    const auto result = marcatili::SolveCouplerPoint(config);

    tests.Expect(
        result.status == "below_transverse_cutoff",
        "coupler missing transverse root should be reported as a physical cutoff"
    );
    tests.Expect(
        result.status_class == "physical_limit",
        "coupler physical cutoff should expose status_class=physical_limit"
    );
    tests.Expect(result.domain_valid, "coupler physical cutoff should keep domain_valid");
    tests.Expect(
        !result.transverse_root_found,
        "coupler physical cutoff should report transverse_root_found=false"
    );
}

void TestCouplerDimensionalOutputs(TestContext& tests) {
    CouplerPointConfig config;
    config.case_id = "coupler_dimensional";
    config.solver_model = SingleGuideSolverModel::kExact;
    config.transverse_equation = CouplerTransverseEquation::kEq6;
    config.p = 1;
    config.a_over_A5 = 1.6;
    config.c_over_a = 1.0;
    config.wavelength = 1.0e-6;
    config.n1 = 1.8;
    config.n5 = 1.5;

    const auto result = marcatili::SolveCouplerPoint(config);

    tests.Expect(result.status == "ok", "dimensional coupler baseline should solve successfully");
    tests.Expect(
        result.dimensional_outputs_available,
        "dimensional coupler baseline should expose dimensional outputs"
    );
    tests.ExpectNear(
        result.A5,
        marcatili::math::ComputeA(config.wavelength, config.n1, config.n5),
        1e-18,
        "dimensional coupler should recover A5 from wavelength and indices"
    );
    tests.Expect(result.coupling_magnitude > 0.0, "dimensional coupler should produce |K|");
    tests.Expect(
        result.full_transfer_length > 0.0,
        "dimensional coupler should produce a positive transfer length"
    );
}

void TestTable1CutoffStatuses(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/reproduce_table1.json"));
    const auto base_config = marcatili::io::ParseTable1Config(input_text, "");

    marcatili::Table1Config found_config = base_config;
    found_config.solver_models = {SingleGuideSolverModel::kExact};
    found_config.rows = {base_config.rows.front()};

    const auto found_result = marcatili::SolveTable1(found_config);
    const auto* found_row =
        FindTable1RowSummary(found_result, base_config.rows.front().row_id, SingleGuideSolverModel::kExact);
    tests.Expect(found_row != nullptr, "table 1 found-status row should exist");
    if (found_row != nullptr) {
        tests.Expect(
            found_row->limiting_cutoff_status == "found",
            "table 1 baseline row should keep the found cutoff status"
        );
        tests.Expect(
            found_row->limiting_cutoff_found,
            "table 1 baseline row should report a numeric cutoff"
        );
    }

    marcatili::Table1Config below_config = found_config;
    below_config.search.b_normalized_min = 20.0;
    below_config.search.b_normalized_max = 30.0;

    const auto below_result = marcatili::SolveTable1(below_config);
    const auto* below_row =
        FindTable1RowSummary(below_result, base_config.rows.front().row_id, SingleGuideSolverModel::kExact);
    tests.Expect(below_row != nullptr, "table 1 below-search-min row should exist");
    if (below_row != nullptr) {
        tests.Expect(
            below_row->limiting_cutoff_status == "below_search_min",
            "table 1 should distinguish cutoff below the searched interval"
        );
        tests.Expect(
            !below_row->limiting_cutoff_found,
            "table 1 cutoff below the search interval should not fabricate a numeric cutoff"
        );
    }

    marcatili::Table1Config above_config = found_config;
    above_config.search.b_normalized_min = 0.01;
    above_config.search.b_normalized_max = 10.0;

    const auto above_result = marcatili::SolveTable1(above_config);
    const auto* above_row =
        FindTable1RowSummary(above_result, base_config.rows.front().row_id, SingleGuideSolverModel::kExact);
    tests.Expect(above_row != nullptr, "table 1 above-search-max row should exist");
    if (above_row != nullptr) {
        tests.Expect(
            above_row->limiting_cutoff_status == "above_search_max",
            "table 1 should distinguish cutoff above the searched interval"
        );
        tests.Expect(
            !above_row->limiting_cutoff_found,
            "table 1 cutoff above the search interval should not fabricate a numeric cutoff"
        );
    }
}

void TestLegacySingleGuideParserCompatibility(TestContext& tests) {
    const auto input_text = marcatili::io::ReadTextFile(
        SourcePath("data/input/solve_single_guide_legacy_flat.json")
    );
    const auto config = marcatili::io::ParseSingleGuideConfig(input_text, "");

    tests.Expect(
        config.solver_model == SingleGuideSolverModel::kClosedForm,
        "legacy flat single-guide input should preserve solver_model parsing"
    );
    tests.Expect(
        config.family == SingleGuideFamily::kEy,
        "legacy flat single-guide input should preserve family parsing"
    );
    tests.Expect(config.p == 1 && config.q == 1, "legacy flat single-guide mode indices should parse");
    tests.ExpectNear(config.wavelength, 1.0e-6, 1e-18, "legacy flat single-guide wavelength should parse");
}

void TestFigure6Regression(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/fig6/SG-006h.json"));
    const auto config = marcatili::io::ParseFigure6Config(input_text, "");
    const auto result = marcatili::SolveFigure6(config);
    const auto* sample =
        FindFigure6Sample(result, "E_y_1_1", SingleGuideSolverModel::kExact, 28);

    tests.Expect(sample != nullptr, "figure 6 regression sample should exist");
    if (sample == nullptr) {
        return;
    }

    tests.Expect(sample->point.guided, "figure 6 regression sample should be guided");
    tests.ExpectNear(
        sample->point.kz_normalized_against_n4,
        0.097233476011134093,
        1e-12,
        "figure 6 regression kz normalization drifted"
    );
}

void TestFigure7Regression(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/reproduce_fig7.json"));
    const auto config = marcatili::io::ParseFigure7Config(input_text, "");
    const auto result = marcatili::SolveFigure7(config);
    const auto* intersection = FindFigure7Intersection(result, "E_y_1_1", "C=25");

    tests.Expect(intersection != nullptr, "figure 7 reference intersection should exist");
    if (intersection == nullptr) {
        return;
    }

    tests.Expect(intersection->guided, "figure 7 reference intersection should stay guided");
    tests.ExpectNear(
        intersection->kz_normalized_against_n4,
        0.96315259645815465,
        1e-12,
        "figure 7 reference intersection drifted"
    );
}

void TestFigure8Regression(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/reproduce_fig8.json"));
    const auto config = marcatili::io::ParseFigure8Config(input_text, "");
    const auto result = marcatili::SolveFigure8(config);
    const auto* sample =
        FindFigure8Sample(result, "E_y_1_1", SingleGuideSolverModel::kExact, 21);

    tests.Expect(sample != nullptr, "figure 8 regression sample should exist");
    if (sample == nullptr) {
        return;
    }

    tests.Expect(sample->point.guided, "figure 8 regression sample should be guided");
    tests.ExpectNear(
        sample->point.kz_normalized_against_n4,
        0.014758337141013173,
        1e-12,
        "figure 8 regression kz normalization drifted"
    );
}

void TestFigure10Regression(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/reproduce_fig10.json"));
    const auto config = marcatili::io::ParseFigure10Config(input_text, "");
    const auto result = marcatili::SolveFigure10(config);
    const auto* sample =
        FindFigure10Sample(result, "a_over_A5=0.5", SingleGuideSolverModel::kExact, 0);

    tests.Expect(sample != nullptr, "figure 10 regression sample should exist");
    if (sample == nullptr) {
        return;
    }

    tests.Expect(
        sample->point.status == "ok",
        "figure 10 regression sample should remain inside the coupling domain"
    );
    tests.ExpectNear(
        sample->point.normalized_coupling,
        0.76908173652611211,
        1e-12,
        "figure 10 normalized coupling drifted"
    );
}

void TestFigure11Regression(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/reproduce_fig11.json"));
    const auto config = marcatili::io::ParseFigure11Config(input_text, "");
    const auto result = marcatili::SolveFigure11(config);
    const auto* sample = FindFigure11Sample(
        result,
        "n1_over_n5=1.5",
        "a_over_A5=0.5",
        SingleGuideSolverModel::kExact,
        0
    );

    tests.Expect(sample != nullptr, "figure 11 regression sample should exist");
    if (sample == nullptr) {
        return;
    }

    tests.Expect(
        sample->point.status == "ok",
        "figure 11 regression sample should remain inside the coupling domain"
    );
    tests.ExpectNear(
        sample->point.normalized_coupling,
        0.63806722185683573,
        1e-12,
        "figure 11 normalized coupling drifted"
    );
}

void TestTable1Regression(TestContext& tests) {
    const auto input_text =
        marcatili::io::ReadTextFile(SourcePath("data/input/reproduce_table1.json"));
    const auto config = marcatili::io::ParseTable1Config(input_text, "");
    const auto result = marcatili::SolveTable1(config);
    const auto* row =
        FindTable1RowSummary(result, "SG-TBL-001A-1001", SingleGuideSolverModel::kExact);

    tests.Expect(row != nullptr, "table 1 regression row should exist");
    if (row == nullptr) {
        return;
    }

    tests.Expect(
        row->limiting_cutoff_status == "found",
        "table 1 regression row should keep status=found"
    );
    tests.ExpectNear(
        row->computed_dimension_normalized,
        16.56538682505489,
        1e-10,
        "table 1 regression dimension drifted"
    );
}

}  // namespace

int main() {
    TestContext tests;

    TestGuidedCriterionUsesCriticalExternalMedium(tests);
    TestExactBelowCutoffClassification(tests);
    TestCouplerNoRootIsPhysicalStatus(tests);
    TestCouplerDimensionalOutputs(tests);
    TestTable1CutoffStatuses(tests);
    TestLegacySingleGuideParserCompatibility(tests);
    TestFigure6Regression(tests);
    TestFigure7Regression(tests);
    TestFigure8Regression(tests);
    TestFigure10Regression(tests);
    TestFigure11Regression(tests);
    TestTable1Regression(tests);

    if (tests.failures != 0) {
        std::cerr << tests.failures << " regression check(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All regression checks passed.\n";
    return EXIT_SUCCESS;
}
