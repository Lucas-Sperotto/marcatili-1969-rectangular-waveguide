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

/**
 * @brief Resolve numericamente uma raiz real pelo método da secante.
 *
 * @param f Função escalar cuja raiz será buscada.
 * @param x0 Primeiro chute inicial.
 * @param x1 Segundo chute inicial.
 * @param tol Tolerância numérica para critério de parada.
 * @param max_iter Número máximo de iterações do método.
 * @param iter_count Ponteiro opcional para receber o número de iterações usadas.
 *
 * @return Aproximação da raiz.
 *
 * @throws std::invalid_argument Se os parâmetros numéricos forem inválidos.
 * @throws std::runtime_error Se a função produzir valor não-finito ou se a
 *         atualização da secante ficar degenerada.
 */
double secant(
    std::function<double(double)> f,
    double x0,
    double x1,
    double tol,
    int max_iter,
    int* iter_count = nullptr
);

/**
 * @brief Resolve numericamente uma raiz pelo método de Newton com derivada numérica.
 *
 * @param f Função escalar cuja raiz será buscada.
 * @param x0 Chute inicial.
 * @param tol Tolerância numérica para critério de parada.
 * @param max_iter Número máximo de iterações do método.
 * @param h Passo usado na diferença central da derivada.
 * @param iter_count Ponteiro opcional para receber o número de iterações usadas.
 *
 * @return Aproximação da raiz.
 *
 * @throws std::invalid_argument Se os parâmetros numéricos forem inválidos.
 * @throws std::runtime_error Se a função produzir valor não-finito ou se a
 *         derivada numérica ficar degenerada.
 */
double newton_fd(
    std::function<double(double)> f,
    double x0,
    double tol,
    int max_iter,
    double h = 1e-7,
    int* iter_count = nullptr
);

/**
 * @brief Resolve numericamente uma raiz pelo método da falsa posição.
 *
 * @param f Função escalar contínua cuja raiz será buscada.
 * @param a Limite inferior do intervalo inicial.
 * @param b Limite superior do intervalo inicial.
 * @param tol Tolerância numérica para critério de parada.
 * @param max_iter Número máximo de iterações do método.
 * @param iter_count Ponteiro opcional para receber o número de iterações usadas.
 *
 * @return Aproximação da raiz no intervalo informado.
 *
 * @throws std::invalid_argument Se os parâmetros numéricos forem inválidos.
 * @throws std::runtime_error Se o intervalo não contiver mudança de sinal ou
 *         se a atualização ficar degenerada.
 */
double false_position(
    std::function<double(double)> f,
    double a,
    double b,
    double tol,
    int max_iter,
    int* iter_count = nullptr
);

/**
 * @brief Resolve uma iteração de ponto fixo p = g(x).
 *
 * @param g Função de iteração de ponto fixo.
 * @param x0 Chute inicial.
 * @param tol Tolerância numérica para critério de parada.
 * @param max_iter Número máximo de iterações do método.
 * @param iter_count Ponteiro opcional para receber o número de iterações usadas.
 *
 * @return Aproximação do ponto fixo.
 *
 * @throws std::invalid_argument Se os parâmetros numéricos forem inválidos.
 * @throws std::runtime_error Se a função produzir valor não-finito.
 */
double fixed_point(
    std::function<double(double)> g,
    double x0,
    double tol,
    int max_iter,
    int* iter_count = nullptr
);

}  // namespace marcatili::math
