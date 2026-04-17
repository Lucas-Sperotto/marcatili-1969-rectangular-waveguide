#include "marcatili/math/waveguide_math.hpp"

#include <algorithm>
#include <cmath>

namespace marcatili::math {

/**
 * @brief Computes the characteristic scaling parameter A_v from Eq. (10).
 * @details This parameter represents a natural length scale for the evanescent
 * field decay, determined by the wavelength and the refractive index contrast
 * between the core (n1) and a surrounding medium (nv).
 */
double ComputeA(double wavelength, double n1, double nv) {
    // A_v = lambda / (2 sqrt(n1^2 - nv^2)) is the geometric scale that appears
    // throughout Marcatili's normalization. In the plots, ratios such as a/A_5
    // and b/A_4 measure the guide dimensions against the evanescent decay scale
    // set by the cladding index on each side.
    return wavelength / (2.0 * std::sqrt(Square(n1) - Square(nv)));
}

/**
 * @brief Computes a safe upper bound for the root-finding interval.
 * @details This is a numerical utility to avoid evaluating functions at the
 * exact boundary of their domain, where they might be singular.
 */
double SafeUpperBound(double a_value, double b_value, double epsilon) {
    // Exact solvers approach singular phase terms when k_t approaches pi / A_v.
    // We stay slightly below the theoretical limit so the transcendental functions
    // remain finite during the root search.
    return std::min(a_value, b_value) * (1.0 - epsilon);
}

/**
 * @brief Computes the penetration depth (xi or eta) from Eq. (8), (9), (18), (19).
 * @details This is the distance into the cladding over which the evanescent
 * field amplitude decays to 1/e of its value at the interface.
 */
double PenetrationDepth(double A_value, double transverse_wave_number) {
    // In the adopted Marcatili model, xi and eta are decay lengths in the outer
    // dielectric regions. After rewriting the cutoff relation with A_v, they can
    // be evaluated directly from the transverse wave number.
    return 1.0 / std::sqrt(Square(kPi / A_value) - Square(transverse_wave_number));
}

}  // namespace marcatili::math
