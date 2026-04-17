#pragma once

namespace marcatili::math {

/**
 * @brief Constante pi usada nas fórmulas do modelo de Marcatili.
 */
constexpr double kPi = 3.14159265358979323846;

/**
 * @brief Retorna o quadrado de um valor escalar.
 */
inline double Square(double value) {
    return value * value;
}

/**
 * @brief Calcula o parâmetro característico A_v.
 *
 * @param wavelength Comprimento de onda no vácuo.
 * @param n1 Índice de refração do núcleo.
 * @param nv Índice de refração do meio v.
 *
 * @return Valor de A_v.
 *
 * @details
 * Corresponde ao fator
 * A_v = lambda / (2 * sqrt(n1^2 - nv^2)),
 * que aparece na normalização geométrica do modelo de Marcatili.
 */
double ComputeA(double wavelength, double n1, double nv);

/**
 * @brief Retorna um limite superior ligeiramente recuado para busca de raiz.
 *
 * @param a_value Primeiro limite candidato.
 * @param b_value Segundo limite candidato.
 * @param epsilon Fração de recuo em relação ao menor limite.
 *
 * @return Limite superior seguro para busca numérica.
 *
 * @details
 * Esta função evita avaliar expressões transcendentais exatamente em fronteiras
 * onde tangentes, raízes ou denominadores podem se tornar singulares.
 */
double SafeUpperBound(double a_value, double b_value, double epsilon = 1e-10);

/**
 * @brief Calcula a profundidade de penetração evanescente.
 *
 * @param A_value Parâmetro A_v associado ao meio externo.
 * @param transverse_wave_number Número de onda transversal.
 *
 * @return Profundidade de penetração 1/e no meio evanescente.
 *
 * @details
 * No modelo adotado, esta rotina é usada para calcular grandezas como
 * xi e eta a partir de relações equivalentes às Eqs. (8), (9), (18) e (19).
 */
double PenetrationDepth(double A_value, double transverse_wave_number);

}  // namespace marcatili::math
