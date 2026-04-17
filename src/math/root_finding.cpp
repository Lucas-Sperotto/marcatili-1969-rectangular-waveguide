#include "marcatili/math/root_finding.hpp"

#include <cmath>
#include <stdexcept>

namespace marcatili::math {

double SolveRootByBisection(
    const std::function<double(double)>& function,
    double lower,
    double upper,
    int max_iterations,
    double tolerance
) {
    double left = lower;
    double right = upper;
    double f_left = function(left);
    double f_right = function(right);

    if (f_left == 0.0) {
        return left;
    }

    if (f_right == 0.0) {
        return right;
    }

    if (f_left * f_right > 0.0) {
        throw std::runtime_error("Bisection interval does not bracket a root.");
    }

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        const double midpoint = 0.5 * (left + right);
        const double f_mid = function(midpoint);

        if (std::abs(f_mid) < tolerance || std::abs(right - left) < tolerance) {
            return midpoint;
        }

        if (f_left * f_mid <= 0.0) {
            right = midpoint;
            f_right = f_mid;
        } else {
            left = midpoint;
            f_left = f_mid;
        }
    }

    return 0.5 * (left + right);
}

}  // namespace marcatili::math
