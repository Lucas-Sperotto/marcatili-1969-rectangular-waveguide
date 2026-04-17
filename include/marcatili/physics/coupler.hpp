#pragma once

#include <string>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Seleciona qual equação transversal do modelo de Marcatili será usada.
 *
 * @details
 * - `kEq6`  : base usada na reprodução da Fig. 10;
 * - `kEq20` : base usada na reprodução da Fig. 11.
 */
enum class CouplerTransverseEquation {
    kEq6,
    kEq20
};

/**
 * @brief Configuração de um ponto do problema normalizado do acoplador.
 *
 * @details
 * Esta estrutura representa um único ponto de avaliação do modelo de
 * acoplamento normalizado. A implementação calcula primeiro a raiz
 * transversal normalizada `u = k_x A_5 / pi` para o guia isolado e,
 * em seguida, aplica a fórmula do acoplamento normalizado.
 */
struct CouplerPointConfig {
    std::string case_id;
    std::string article_target;
    std::string csv_output_path;

    // Modelo do solver transversal herdado do problema de guia único.
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;

    // Eq. (6) é usada como base da Fig. 10; Eq. (20), da Fig. 11.
    CouplerTransverseEquation transverse_equation = CouplerTransverseEquation::kEq6;

    // Índice modal horizontal p na equação transversal.
    int p = 1;

    // Largura elétrica normalizada: a / A_5.
    double a_over_A5 = 0.0;

    // Separação horizontal normalizada pela largura do guia: c / a.
    double c_over_a = 0.0;

    // Relação auxiliar do caso simétrico: r = (n_5 / n_1)^2.
    // Necessária para Eq. (20) e para a aproximação correspondente.
    double index_ratio_squared = 0.0;
};

/**
 * @brief Resultado de um ponto resolvido do acoplador.
 */
struct CouplerPointResult {
    CouplerPointConfig config;
    bool domain_valid = false;
    std::string status;
    std::string equations_used;

    double a_over_A5 = 0.0;
    double c_over_A5 = 0.0;
    double kx_A5_over_pi = 0.0;
    double sqrt_one_minus_kx_A5_over_pi_squared = 0.0;
    double normalized_coupling = 0.0;
};

/**
 * @brief Converte a enumeração da equação transversal para texto.
 */
std::string ToString(CouplerTransverseEquation equation);

/**
 * @brief Converte texto para a enumeração da equação transversal.
 *
 * @throws std::invalid_argument Se o texto não representar uma opção suportada.
 */
CouplerTransverseEquation ParseCouplerTransverseEquation(const std::string& equation_text);

/**
 * @brief Resolve um ponto do modelo normalizado do acoplador.
 *
 * @details
 * O cálculo ocorre em duas etapas:
 * 1. resolve-se a raiz transversal normalizada `u = k_x A_5 / pi`;
 * 2. aplica-se a expressão do acoplamento normalizado em função de `u`
 *    e de `c / A_5`.
 *
 * @throws std::invalid_argument Se a configuração de entrada for inválida.
 * @throws std::runtime_error Se a busca da raiz falhar numericamente.
 */
CouplerPointResult SolveCouplerPoint(const CouplerPointConfig& config);

}  // namespace marcatili
