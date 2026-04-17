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
    if (max_iterations <= 0) {
        throw std::invalid_argument("SolveRootByBisection: max_iterations must be positive.");
    }

    if (tolerance <= 0.0) {
        throw std::invalid_argument("SolveRootByBisection: tolerance must be positive.");
    }

    if (lower > upper) {
        throw std::invalid_argument("SolveRootByBisection: lower bound must not exceed upper bound.");
    }

    double left = lower;
    double right = upper;

    double f_left = function(left);
    double f_right = function(right);

    if (!std::isfinite(f_left) || !std::isfinite(f_right)) {
        throw std::runtime_error(
            "SolveRootByBisection: function returned a non-finite value at an interval endpoint."
        );
    }

    if (f_left == 0.0) {
        return left;
    }

    if (f_right == 0.0) {
        return right;
    }

    // O método da bisseção exige que a raiz esteja isolada por mudança de sinal.
    if (f_left * f_right > 0.0) {
        throw std::runtime_error(
            "SolveRootByBisection: interval does not bracket a root."
        );
    }

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        const double midpoint = 0.5 * (left + right);
        const double f_mid = function(midpoint);

        if (!std::isfinite(f_mid)) {
            throw std::runtime_error(
                "SolveRootByBisection: function returned a non-finite value inside the interval."
            );
        }

        // Critérios de parada:
        // 1. resíduo suficientemente pequeno;
        // 2. intervalo suficientemente estreito.
        if (std::abs(f_mid) < tolerance || std::abs(right - left) < tolerance) {
            return midpoint;
        }

        // Mantemos apenas o subintervalo que preserva a mudança de sinal.
        if (f_left * f_mid <= 0.0) {
            right = midpoint;
            f_right = f_mid;
        } else {
            left = midpoint;
            f_left = f_mid;
        }
    }

    // Se o limite de iterações for atingido, retornamos o melhor ponto médio obtido.
    return 0.5 * (left + right);
}

}  // namespace marcatili::math
