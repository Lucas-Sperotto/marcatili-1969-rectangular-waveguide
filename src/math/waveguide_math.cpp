#include "marcatili/math/waveguide_math.hpp"

#include <algorithm>
#include <cmath>

namespace marcatili::math {

double ComputeA(double wavelength, double n1, double nv) {
    return wavelength / (2.0 * std::sqrt(Square(n1) - Square(nv)));
}

double SafeUpperBound(double a_value, double b_value, double epsilon) {
    return std::min(a_value, b_value) * (1.0 - epsilon);
}

double PenetrationDepth(double A_value, double transverse_wave_number) {
    return 1.0 / std::sqrt(Square(kPi / A_value) - Square(transverse_wave_number));
}

}  // namespace marcatili::math
