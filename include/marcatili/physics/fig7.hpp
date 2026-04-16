#pragma once

#include <limits>
#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

struct Figure7ModeSpec {
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    std::string line_id;
};

struct Figure7CLineSpec {
    double c_value = 0.0;
    std::string line_id;
};

struct Figure7Config {
    std::string case_id;
    std::string article_target;
    std::string lines_csv_output_path;
    std::string intersections_csv_output_path;
    std::string article_reference_mode_line_id;
    std::string article_reference_note;
    double wavelength = 0.0;
    double a = 0.0;
    double b = 0.0;
    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;
    int line_point_count = 0;
    double reference_c_value = 0.0;
    double article_reference_y_readoff = std::numeric_limits<double>::quiet_NaN();
    std::vector<Figure7ModeSpec> modes;
    std::vector<Figure7CLineSpec> c_lines;
};

struct Figure7LineSample {
    std::string line_kind;
    std::string line_id;
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 0;
    int q = 0;
    double c_value = 0.0;
    int sample_index = 0;
    double x = 0.0;
    double y = 0.0;
};

struct Figure7Intersection {
    std::string mode_line_id;
    std::string c_line_id;
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    double c_value = 0.0;
    double x = 0.0;
    double y = 0.0;
    bool is_reference_c = false;
    bool domain_valid = false;
    bool guided = false;
    double kz = 0.0;
    double kz_normalized_against_n4 = 0.0;
};

struct Figure7DesignExampleSummary {
    bool symmetric_material_pairs = false;
    double a_over_b = 0.0;
    double delta_from_n35 = std::numeric_limits<double>::quiet_NaN();
    double delta_prime_from_n24 = std::numeric_limits<double>::quiet_NaN();
    double sqrt_delta_prime_over_delta = std::numeric_limits<double>::quiet_NaN();
};

struct Figure7ArticleReferenceCheck {
    bool available = false;
    std::string mode_line_id;
    std::string note;
    double c_value = std::numeric_limits<double>::quiet_NaN();
    double exact_x = std::numeric_limits<double>::quiet_NaN();
    double exact_y = std::numeric_limits<double>::quiet_NaN();
    double article_y_readoff = std::numeric_limits<double>::quiet_NaN();
    double article_y_absolute_error = std::numeric_limits<double>::quiet_NaN();
    double article_y_relative_error = std::numeric_limits<double>::quiet_NaN();
};

struct Figure7Result {
    Figure7Config config;
    std::string status;
    double k0 = 0.0;
    double k1 = 0.0;
    double k2 = 0.0;
    double k3 = 0.0;
    double k4 = 0.0;
    double k5 = 0.0;
    double A2 = 0.0;
    double A3 = 0.0;
    double A4 = 0.0;
    double A5 = 0.0;
    double x_numerator = 0.0;
    double y_numerator = 0.0;
    double derived_c = 0.0;
    Figure7DesignExampleSummary design_example;
    Figure7ArticleReferenceCheck article_reference_check;
    std::vector<Figure7LineSample> line_samples;
    std::vector<Figure7Intersection> intersections;
};

Figure7ModeSpec ParseFigure7ModeSpec(const std::string& mode_text);
Figure7CLineSpec ParseFigure7CLineSpec(const std::string& c_text);
Figure7Result SolveFigure7(const Figure7Config& config);

}  // namespace marcatili
