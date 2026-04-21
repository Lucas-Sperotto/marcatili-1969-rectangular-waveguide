#include "marcatili/math/waveguide_math.hpp"

// ============================================================================
// Implementação das rotinas matemáticas de suporte — Modelo de Marcatili
// ============================================================================
//
// As três funções deste arquivo correspondem às seguintes equações do artigo:
//
//   ComputeA(λ, n₁, nᵥ)
//     → Eq. (10) [família E^y] e Eq. (18) [família E^x]:
//        A_v = λ / ( 2 √(n₁² − nᵥ²) )
//        Parâmetro geométrico de escala para o decaimento evanescente no meio v.
//
//   PenetrationDepth(A_v, k_t)
//     → Eqs. (8)–(9) [família E^y] e Eqs. (18)–(19) [família E^x]:
//        ξ ou η = [ (π/A_v)² − k_t² ]^{−½}
//        Distância 1/e de decaimento do campo evanescente no meio externo v.
//
//   SafeUpperBound(a, b, ε)
//     → Utilitário numérico sem correspondência direta no artigo.
//        Retorna min(a,b)·(1−ε) para evitar singularidades nos extremos
//        do intervalo de busca das equações transcendentais.
//
// Referência: Marcatili, E. A. J., Bell Syst. Tech. J. 48, 2071–2102 (1969).
// ============================================================================

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace marcatili::math {

// ─────────────────────────────────────────────────────────────────────────────
// ComputeA
// ─────────────────────────────────────────────────────────────────────────────
//
// Eq. (10) [família E^y] e Eq. (18) [família E^x]:
//
//               λ
//   A_v = ─────────────────────
//          2 √( n₁² − nᵥ² )
//
// Onde:
//   λ   = comprimento de onda no vácuo
//   n₁  = índice de refração do núcleo (região 1 da Fig. 3)
//   nᵥ  = índice do meio externo v ∈ {2, 3, 4, 5}
//
// A_v tem unidade de comprimento e é a escala característica do decaimento
// evanescente no meio v. Quanto maior o contraste (n₁² − nᵥ²), menor A_v
// e mais confinado o modo ao núcleo.
//
// Uso típico em BuildBaseResult (single_guide.cpp):
//   result.A2 = ComputeA(λ, n₁, n₂)  →  escala na face inferior
//   result.A3 = ComputeA(λ, n₁, n₃)  →  escala na face esquerda
//   result.A4 = ComputeA(λ, n₁, n₄)  →  escala na face superior
//   result.A5 = ComputeA(λ, n₁, n₅)  →  escala na face direita
//
// A_v também aparece como:
//   - limite superior da busca de raiz: k_t < π/A_v  (cutoff transversal)
//   - normalizador do eixo horizontal: b/A₄ (Fig. 6), a/A₅ (Figs. 10–11)
//   - fator de conversão no acoplador: k_x = u·π/A₅  onde u = k_x A₅/π
// ─────────────────────────────────────────────────────────────────────────────
double ComputeA(double wavelength, double n1, double nv) {
    if (wavelength <= 0.0) {
        throw std::invalid_argument("ComputeA: wavelength must be positive.");
    }

    if (n1 <= 0.0 || nv <= 0.0) {
        throw std::invalid_argument("ComputeA: refractive indices must be positive.");
    }

    // n₁² − nᵥ²  (contraste de índice ao quadrado)
    // Deve ser positivo: o núcleo precisa ter índice maior que o meio externo
    // para que haja reflexão interna total e guiamento do modo.
    const double contrast = Square(n1) - Square(nv);
    if (contrast <= 0.0) {
        throw std::invalid_argument(
            "ComputeA: requires n1^2 - nv^2 > 0 (núcleo deve ter índice maior que o meio v)."
        );
    }

    // A_v = λ / ( 2 √(n₁² − nᵥ²) )   →   Eq. (10) / Eq. (18)
    return wavelength / (2.0 * std::sqrt(contrast));
}

// ─────────────────────────────────────────────────────────────────────────────
// SafeUpperBound
// ─────────────────────────────────────────────────────────────────────────────
//
// As equações transcendentais (6), (7), (20), (21) têm a forma:
//
//   f(k_t) = k_t·d + atan(k_t·ξ_j) + atan(k_t·ξ_k) − pπ = 0
//
// onde ξ_j = PenetrationDepth(A_j, k_t) diverge quando k_t → π/A_j.
// Nesse ponto, atan(k_t·ξ_j) → atan(+∞) = π/2 e a função f tem uma
// descontinuidade aparente que impede a bisseção de funcionar corretamente.
//
// A solução é definir o limite superior do intervalo como:
//
//   upper = min(π/A_j, π/A_k) · (1 − ε)
//
// O recuo de ε garante que PenetrationDepth seja avaliável (radicando > 0)
// nos dois extremos do intervalo de busca.
// ─────────────────────────────────────────────────────────────────────────────
double SafeUpperBound(double a_value, double b_value, double epsilon) {
    if (epsilon < 0.0 || epsilon >= 1.0) {
        throw std::invalid_argument(
            "SafeUpperBound: epsilon must satisfy 0 <= epsilon < 1."
        );
    }

    // Tomamos o mínimo dos dois candidatos: min(π/A_j, π/A_k)
    // Este é o cutoff transversal mais restritivo.
    const double upper_bound = std::min(a_value, b_value);

    if (upper_bound <= 0.0) {
        throw std::invalid_argument(
            "SafeUpperBound: upper bound candidates must define a positive interval."
        );
    }

    // Recuo de ε: afasta o limite superior do ponto de singularidade
    return upper_bound * (1.0 - epsilon);
}

// ─────────────────────────────────────────────────────────────────────────────
// PenetrationDepth
// ─────────────────────────────────────────────────────────────────────────────
//
// Para a família E^y — Eqs. (8) e (9):
//
//   ξ₃ = [ (π/A₃)² − k_x² ]^{−½}   (decaimento na face esquerda, meio 3)
//   ξ₅ = [ (π/A₅)² − k_x² ]^{−½}   (decaimento na face direita,  meio 5)
//   η₂ = [ (π/A₂)² − k_y² ]^{−½}   (decaimento na face inferior, meio 2)
//   η₄ = [ (π/A₄)² − k_y² ]^{−½}   (decaimento na face superior, meio 4)
//
// Para a família E^x — Eqs. (18) e (19), a mesma estrutura formal se aplica
// (a diferença entre as famílias está nos fatores n²/n₁² das equações
// transcendentais, não na expressão de ξ ou η em si).
//
// Condição de modo guiado: k_t < π/A_v  →  radicando > 0  →  decaimento real
// Modo não guiado:         k_t ≥ π/A_v  →  radicando ≤ 0  →  exceção
//
// Esta função é chamada dentro das lambdas de resíduo fx e fy em
// single_guide.cpp, a cada iteração da bisseção, com o valor tentativo
// de k_t sendo avaliado pelo solver.
// ─────────────────────────────────────────────────────────────────────────────
double PenetrationDepth(double A_value, double transverse_wave_number) {
    if (A_value <= 0.0) {
        throw std::invalid_argument("PenetrationDepth: A_value must be positive.");
    }

    // Radicando: (π/A_v)² − k_t²
    // Deve ser estritamente positivo para que o campo seja evanescente no meio v.
    // Se k_t ≥ π/A_v, o modo está abaixo do cutoff transversal nessa direção.
    const double radicand = Square(kPi / A_value) - Square(transverse_wave_number);

    if (radicand <= 0.0) {
        throw std::invalid_argument(
            "PenetrationDepth: requires (pi/A_v)^2 - k_t^2 > 0 (modo não guiado nesta direção)."
        );
    }

    // ξ ou η = 1 / √( (π/A_v)² − k_t² )   →   Eqs. (8)–(9) / Eqs. (18)–(19)
    return 1.0 / std::sqrt(radicand);
}

}  // namespace marcatili::math
