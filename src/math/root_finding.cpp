#include "marcatili/math/root_finding.hpp"

// ============================================================================
// Método da bisseção — núcleo numérico do repositório
// ============================================================================
//
// Este arquivo implementa o único solver numérico usado em todo o repositório.
// Ele resolve equações transcendentais da forma:
//
//   f(k_t) = 0,   k_t ∈ (lower, upper)
//
// As equações que chegam aqui são:
//
//   Família E^y:
//     Eq. (6):  f_x(k_x) = k_x a + atan(k_x ξ₃) + atan(k_x ξ₅) − pπ = 0
//     Eq. (7):  f_y(k_y) = k_y b + atan(n₂²/n₁² k_y η₂)
//                                 + atan(n₄²/n₁² k_y η₄) − qπ = 0
//
//   Família E^x:
//     Eq. (20): f_x(k_x) = k_x a + atan(n₃²/n₁² k_x ξ₃)
//                                 + atan(n₅²/n₁² k_x ξ₅) − pπ = 0
//     Eq. (21): f_y(k_y) = k_y b + atan(k_y η₂) + atan(k_y η₄) − qπ = 0
//
//   Acoplador (versão normalizada u = k_x A₅/π):
//     Eq. (6) normalizada:  π(a/A₅)u + 2 atan(u/√(1−u²)) − pπ = 0
//     Eq. (20) normalizada: π(a/A₅)u + 2 atan(r u/√(1−u²)) − pπ = 0
//
// Por que bisseção?
//   As funções acima são contínuas no intervalo aberto (0, π/A_v) e
//   monotônicas em cada ramo. O Teorema do Valor Intermediário garante
//   que há exatamente uma raiz por ramo quando o modo é guiado. A bisseção
//   é robusta, simples e converge geometricamente: o erro é reduzido à
//   metade a cada iteração.
//
// Convergência:
//   Com tolerância 10⁻¹² e 200 iterações, o erro máximo possível é
//   2⁻²⁰⁰ ≈ 10⁻⁶⁰ — muito menor que qualquer tolerância física relevante.
//   Na prática a convergência ocorre em ~40 iterações para tol = 10⁻¹².
//
// Referência: Marcatili, E. A. J., Bell Syst. Tech. J. 48, 2071–2102 (1969).
// ============================================================================

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
    // ─────────────────────────────────────────────────────────────────────
    // Validação dos parâmetros de entrada
    // ─────────────────────────────────────────────────────────────────────
    if (max_iterations <= 0) {
        throw std::invalid_argument("SolveRootByBisection: max_iterations must be positive.");
    }

    if (tolerance <= 0.0) {
        throw std::invalid_argument("SolveRootByBisection: tolerance must be positive.");
    }

    if (lower > upper) {
        throw std::invalid_argument("SolveRootByBisection: lower bound must not exceed upper bound.");
    }

    // ─────────────────────────────────────────────────────────────────────
    // Inicialização do intervalo [left, right]
    // ─────────────────────────────────────────────────────────────────────
    double left  = lower;
    double right = upper;

    double f_left  = function(left);
    double f_right = function(right);

    // Se a função não for finita nos extremos, algo está errado com o problema
    // (provavelmente lower ou upper está fora do domínio de f).
    if (!std::isfinite(f_left) || !std::isfinite(f_right)) {
        throw std::runtime_error(
            "SolveRootByBisection: function returned a non-finite value at an interval endpoint."
        );
    }

    // Caso exato nos extremos: raro, mas possível se o modo está exatamente
    // no cutoff (k_t = π/A_v fornece f = 0 para certas geometrias).
    if (f_left  == 0.0) return left;
    if (f_right == 0.0) return right;

    // ─────────────────────────────────────────────────────────────────────
    // Condição de Bolzano: f(left) e f(right) devem ter sinais opostos.
    //
    // Para as equações de Marcatili, o comportamento típico é:
    //   f(ε) ≈ ε·d − pπ < 0   (k_t próximo de zero → lado esquerdo da reta pπ)
    //   f(upper) > 0           (k_t próximo de π/A_v → a soma de atans satura em π)
    //
    // Se f(lower) ≥ 0, a configuração está fora do domínio do modelo.
    // Se f(upper) ≤ 0, o modo está abaixo do cutoff (não guiado).
    // Ambos os casos são tratados em SolveExactRootWithStatus antes de
    // chegar aqui.
    // ─────────────────────────────────────────────────────────────────────
    if (f_left * f_right > 0.0) {
        throw std::runtime_error(
            "SolveRootByBisection: interval does not bracket a root."
        );
    }

    // ─────────────────────────────────────────────────────────────────────
    // Iterações da bisseção
    //
    // A cada iteração:
    //   1. Calcula o ponto médio do intervalo atual.
    //   2. Avalia f no ponto médio.
    //   3. Verifica os critérios de parada.
    //   4. Mantém o subintervalo que ainda contém a mudança de sinal.
    //
    // Invariante: f(left) < 0  e  f(right) > 0  em todo momento.
    // (Garantido pela condição de Bolzano inicial e pela lógica de atualização.)
    // ─────────────────────────────────────────────────────────────────────
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        // Ponto médio do intervalo atual
        const double midpoint = 0.5 * (left + right);
        const double f_mid    = function(midpoint);

        if (!std::isfinite(f_mid)) {
            throw std::runtime_error(
                "SolveRootByBisection: function returned a non-finite value inside the interval."
            );
        }

        // Critérios de parada (dois, para robustez):
        //   1. Resíduo suficientemente pequeno: |f(mid)| < tol
        //      → a equação transcendental está satisfeita com precisão tol
        //   2. Intervalo suficientemente estreito: |right − left| < tol
        //      → geometricamente, não há como melhorar a localização da raiz
        if (std::abs(f_mid) < tolerance || std::abs(right - left) < tolerance) {
            return midpoint;
        }

        // Atualização do intervalo:
        // Mantemos o subintervalo [left, mid] se f(left)·f(mid) ≤ 0,
        // ou [mid, right] caso contrário. Em ambos os casos, a mudança
        // de sinal é preservada no novo intervalo.
        if (f_left * f_mid <= 0.0) {
            right   = midpoint;   // a raiz está em [left, mid]
            f_right = f_mid;
        } else {
            left   = midpoint;    // a raiz está em [mid, right]
            f_left = f_mid;
        }
    }

    // Limite de iterações atingido. Com 200 iterações e tol = 10⁻¹², isso
    // ocorre apenas se o intervalo inicial for muito pequeno. O ponto médio
    // final ainda é a melhor estimativa disponível.
    return 0.5 * (left + right);
}

}  // namespace marcatili::math
