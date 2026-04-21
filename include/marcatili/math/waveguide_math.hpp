#pragma once

// ============================================================================
// Utilitários matemáticos do modelo de Marcatili (1969)
// ============================================================================
//
// Este header define as rotinas numéricas de baixo nível reutilizadas em
// todo o repositório. Cada função corresponde diretamente a um bloco de
// equações do artigo:
//
//   ComputeA         →  Eq. (10) para E^y,  Eq. (18) para E^x
//                        A_v = λ / ( 2 √(n₁² − nᵥ²) )
//
//   PenetrationDepth →  Eqs. (8)–(9) para E^y,  Eqs. (18)–(19) para E^x
//                        ξ ou η = [ (π/A_v)² − k_t² ]^{−½}
//
//   SafeUpperBound   →  utilitário numérico para evitar singularidades
//                        nas fronteiras do intervalo de busca de raiz
//
// Referência: Marcatili, E. A. J., "Dielectric rectangular waveguide and
// directional coupler for integrated optics," Bell Syst. Tech. J.,
// vol. 48, pp. 2071–2102, Sep. 1969.
// ============================================================================

namespace marcatili::math {

// ─────────────────────────────────────────────────────────────────────────────
// Constante π com precisão dupla.
// Usada em todas as fórmulas do modelo: k₀ = 2π/λ, kₜ·d = pπ, etc.
// ─────────────────────────────────────────────────────────────────────────────
constexpr double kPi = 3.14159265358979323846;

// ─────────────────────────────────────────────────────────────────────────────
// Square(x) — retorna x².
//
// Utilitário inline para evitar repetição de x*x em expressões como
// k₁² − k_x² − k_y², (π/A_v)² − k_t², etc. Usado extensivamente para
// melhorar legibilidade sem custo de desempenho.
// ─────────────────────────────────────────────────────────────────────────────
inline double Square(double value) {
    return value * value;
}

/**
 * @brief Calcula o parâmetro geométrico de escala A_v do modelo de Marcatili.
 *
 * @param wavelength  Comprimento de onda no vácuo λ (metros).
 * @param n1          Índice de refração do núcleo — região 1 da Fig. 3.
 * @param nv          Índice de refração do meio externo v ∈ {2, 3, 4, 5}.
 *
 * @return A_v em metros.
 *
 * @details
 * Definição — Eq. (10) para a família E^y; Eq. (18) para E^x:
 *
 *                    λ
 *   A_v = ─────────────────────
 *          2 √( n₁² − nᵥ² )
 *
 * Interpretação física:
 *   A_v é a escala de comprimento que governa o decaimento evanescente do
 *   campo no meio externo v. Quanto maior o contraste (n₁² − nᵥ²), menor
 *   A_v e mais rápido o decaimento. Um modo bem guiado tem:
 *     a/A₃ ≪ 1  e  a/A₅ ≪ 1   (campo confinado em x)
 *     b/A₂ ≪ 1  e  b/A₄ ≪ 1   (campo confinado em y)
 *
 * Usos no repositório:
 *   - BuildBaseResult calcula A2, A3, A4, A5 para cada meio externo;
 *   - Normalização dos eixos: b/A₄ (Fig. 6), a/A₅ (Figs. 10–11);
 *   - Limite superior do intervalo de busca: k_t < π/A_v (cutoff transversal);
 *   - Conversão u → k_x: k_x = u · π / A₅  (acoplador, Eq. 34).
 *
 * @throws std::invalid_argument Se λ ≤ 0, n₁ ≤ 0, nᵥ ≤ 0 ou n₁ ≤ nᵥ.
 */
double ComputeA(double wavelength, double n1, double nv);

/**
 * @brief Retorna um limite superior recuado para busca numérica de raiz.
 *
 * @param a_value  Primeiro candidato ao limite superior (tipicamente π/A_j).
 * @param b_value  Segundo candidato ao limite superior (tipicamente π/A_k).
 * @param epsilon  Fração de recuo em relação ao mínimo dos dois candidatos.
 *
 * @return min(a_value, b_value) × (1 − ε).
 *
 * @details
 * As equações transcendentais (6), (7), (20), (21) têm a forma geral:
 *
 *   f(k_t) = k_t · d + atan( k_t · ξ_j ) + atan( k_t · ξ_k ) − pπ = 0
 *
 * onde ξ_j = PenetrationDepth(A_j, k_t) diverge quando k_t → π/A_j.
 *
 * Por isso, o limite superior da bisseção é recuado por ε antes de iniciar
 * a busca, garantindo que a função seja finita e avaliável nos dois extremos.
 *
 * Valor padrão ε = 10⁻¹⁰ é suficientemente pequeno para não distorcer
 * a localização da raiz e suficientemente grande para evitar overflow.
 */
double SafeUpperBound(double a_value, double b_value, double epsilon = 1e-10);

/**
 * @brief Calcula a profundidade de penetração evanescente 1/e no meio externo.
 *
 * @param A_value                Parâmetro A_v do meio externo (metros).
 * @param transverse_wave_number Número de onda transversal k_t (rad/m).
 *
 * @return Profundidade de penetração em metros.
 *
 * @details
 * Para a família E^y — Eqs. (8) e (9):
 *
 *   ξ₃ = [ (π/A₃)² − k_x² ]^{−½}    (decaimento evanescente no meio 3)
 *   ξ₅ = [ (π/A₅)² − k_x² ]^{−½}    (decaimento evanescente no meio 5)
 *   η₂ = [ (π/A₂)² − k_y² ]^{−½}    (decaimento evanescente no meio 2)
 *   η₄ = [ (π/A₄)² − k_y² ]^{−½}    (decaimento evanescente no meio 4)
 *
 * Para a família E^x — Eqs. (18) e (19), a mesma estrutura se aplica
 * com as devidas substituições de índices e subscritos.
 *
 * A função é chamada dentro das lambdas de resíduo com o valor tentativo
 * k_t = kx_value ou ky_value durante a busca da raiz (bisseção).
 *
 * Interpretação física:
 *   A profundidade de penetração é a distância na qual a amplitude do campo
 *   evanescente decai por um fator 1/e além da interface núcleo/revestimento.
 *   Um modo bem guiado tem ξ, η ≪ a, b (campo concentrado no núcleo).
 *
 * Condição necessária: k_t < π/A_v  (modo guiado → campo evanescente).
 * Se k_t ≥ π/A_v o radicando é ≤ 0 e o modo não é guiado naquela direção.
 *
 * @throws std::invalid_argument Se A_value ≤ 0 ou (π/A_v)² ≤ k_t².
 */
double PenetrationDepth(double A_value, double transverse_wave_number);

}  // namespace marcatili::math
