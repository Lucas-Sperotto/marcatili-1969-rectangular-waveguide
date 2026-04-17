#include "marcatili/math/root_finding.hpp"

#include <cmath>
#include <stdexcept>

namespace marcatili::math {

/**
 * @brief Finds the root of a 1D function using the bisection method.
 * @details This is the numerical core for the "exact" solvers. It finds the
 * root of transcendental equations like Eq. (6), (7), (20), and (21) from
 * Marcatili's paper by iteratively narrowing an interval that brackets a root.
 * The term "exact" refers to the numerical solution of Marcatili's reduced
 * model, not a full-vector solution of the complete 2D boundary-value problem.
 */
double SolveRootByBisection(
    const std::function<double(double)>& function,
    double lower,
    double upper,
    int max_iterations,
    double tolerance
) {
    // The "exact" solvers in this repository still solve the reduced Marcatili model,
    // not the full vector 2D boundary-value problem. Their numerical core is therefore
    // a robust one-dimensional root finder applied to transcendental equations such as
    // Eq. (6), Eq. (7), Eq. (20) and Eq. (21).
    // The "exact" solvers in this repository solve the reduced Marcatili model,
    // not the full vector 2D boundary-value problem. Their numerical core is therefore
    // a robust one-dimensional root finder applied to transcendental equations such as
    // Eq. (6), Eq. (7), Eq. (20) and Eq. (21).
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
        // Bisection only works when the caller already isolated a sign change.
        // Keeping this contract explicit helps separate physical modeling from
        // numerical failure: if the bracket is invalid, the issue is upstream.
        throw std::runtime_error("Bisection interval does not bracket a root.");
    }

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        const double midpoint = 0.5 * (left + right);
        const double f_mid = function(midpoint);

        if (std::abs(f_mid) < tolerance || std::abs(right - left) < tolerance) {
            return midpoint;
        }

        // Once the root is bracketed, each step keeps only the half-interval
        // that still contains the sign change.
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
