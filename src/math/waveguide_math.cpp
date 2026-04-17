#include "marcatili/math/waveguide_math.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace marcatili::math {

double ComputeA(double wavelength, double n1, double nv) {
    if (wavelength <= 0.0) {
        throw std::invalid_argument("ComputeA: wavelength must be positive.");
    }

    if (n1 <= 0.0 || nv <= 0.0) {
        throw std::invalid_argument("ComputeA: refractive indices must be positive.");
    }

    const double contrast = Square(n1) - Square(nv);
    if (contrast <= 0.0) {
        throw std::invalid_argument(
            "ComputeA: requires n1^2 - nv^2 > 0."
        );
    }

    // A_v é a escala geométrica associada ao decaimento evanescente no meio v.
    return wavelength / (2.0 * std::sqrt(contrast));
}

double SafeUpperBound(double a_value, double b_value, double epsilon) {
    if (epsilon < 0.0 || epsilon >= 1.0) {
        throw std::invalid_argument(
            "SafeUpperBound: epsilon must satisfy 0 <= epsilon < 1."
        );
    }

    const double upper_bound = std::min(a_value, b_value);

    if (upper_bound <= 0.0) {
        throw std::invalid_argument(
            "SafeUpperBound: upper bound candidates must define a positive interval."
        );
    }

    // Recuamos ligeiramente do limite teórico para evitar singularidades numéricas.
    return upper_bound * (1.0 - epsilon);
}

double PenetrationDepth(double A_value, double transverse_wave_number) {
    if (A_value <= 0.0) {
        throw std::invalid_argument("PenetrationDepth: A_value must be positive.");
    }

    const double radicand = Square(kPi / A_value) - Square(transverse_wave_number);

    if (radicand <= 0.0) {
        throw std::invalid_argument(
            "PenetrationDepth: requires (pi / A_value)^2 - k_t^2 > 0."
        );
    }

    // A profundidade de penetração é a distância na qual a amplitude cai para 1/e.
    return 1.0 / std::sqrt(radicand);
}

}  // namespace marcatili::math
