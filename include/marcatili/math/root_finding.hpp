#pragma once

#include <functional>

namespace marcatili::math {

/**
 * @brief Resolve numericamente uma raiz real em um intervalo fechado por bisseção.
 *
 * @param function Função escalar contínua cuja raiz será buscada.
 * @param lower Limite inferior do intervalo inicial.
 * @param upper Limite superior do intervalo inicial.
 * @param max_iterations Número máximo de iterações do método.
 * @param tolerance Tolerância numérica para critério de parada.
 *
 * @return Aproximação da raiz no intervalo informado.
 *
 * @throws std::invalid_argument Se os parâmetros numéricos forem inválidos.
 * @throws std::runtime_error Se o intervalo não contiver mudança de sinal.
 *
 * @details
 * Este é o núcleo numérico usado pelos solvers `exact` do repositório.
 * Aqui, `exact` significa resolver numericamente as equações transcendentais
 * do modelo reduzido de Marcatili, e não o problema vetorial 2D completo.
 *
 * O método da bisseção exige que o intervalo inicial [lower, upper]
 * contenha uma mudança de sinal da função.
 */
double SolveRootByBisection(
    const std::function<double(double)>& function,
    double lower,
    double upper,
    int max_iterations = 200,
    double tolerance = 1e-12
);

}  // namespace marcatili::math
