#pragma once

namespace marcatili::math {

constexpr double kPi = 3.14159265358979323846;

inline double Square(double value) {
    return value * value;
}

double ComputeA(double wavelength, double n1, double nv);
double SafeUpperBound(double a_value, double b_value, double epsilon = 1e-10);
double PenetrationDepth(double A_value, double transverse_wave_number);

}  // namespace marcatili::math
