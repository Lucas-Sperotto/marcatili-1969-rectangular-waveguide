#pragma once

#include <functional>

namespace marcatili::math {

double SolveRootByBisection(
    const std::function<double(double)>& function,
    double lower,
    double upper,
    int max_iterations = 200,
    double tolerance = 1e-12
);

}  // namespace marcatili::math
