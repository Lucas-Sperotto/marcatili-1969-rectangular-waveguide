#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/coupler.hpp"

namespace marcatili {

/**
 * @brief Especificação de uma família de curvas da Fig. 11.
 *
 * @details
 * Cada curva é definida por um valor fixo de `a / A_5` e é varrida
 * ao longo de `c / a`.
 */
struct Figure11CurveSpec {
    double a_over_A5 = 0.0;
    std::string curve_id;
    std::string label;
};

/**
 * @brief Especificação de uma família de razão de índices da Fig. 11.
 *
 * @details
 * A implementação usa:
 * - `n1_over_n5` como valor mais diretamente legível no relatório;
 * - `index_ratio_squared = (n5 / n1)^2` como parâmetro operacional
 *   necessário para a equação transversal correspondente.
 */
struct Figure11IndexRatioSpec {
    double n1_over_n5 = 0.0;
    double index_ratio_squared = 0.0;
    std::string ratio_id;
    std::string label;
};

/**
 * @brief Configuração completa da reprodução da Fig. 11.
 *
 * @details
 * A Fig. 11 organiza famílias em dois eixos discretos:
 * - curvas com `a / A_5` fixo;
 * - grupos com razão de índices fixa.
 *
 * Em cada combinação dessas famílias, o sweep é feito ao longo de `c / a`.
 */
struct Figure11Config {
    std::string case_id;
    std::string article_target;
    std::string ocr_note;
    std::string csv_output_path;

    double c_over_a_min = 0.0;
    double c_over_a_max = 0.0;
    int point_count = 0;

    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Figure11CurveSpec> curves;
    std::vector<Figure11IndexRatioSpec> index_ratios;
};

/**
 * @brief Resumo por família calculada na reprodução da Fig. 11.
 */
struct Figure11CurveSummary {
    Figure11CurveSpec curve;
    Figure11IndexRatioSpec index_ratio;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kExact;
    int total_points = 0;
    int valid_points = 0;
    double kx_A5_over_pi = 0.0;
};

/**
 * @brief Amostra individual produzida ao longo do sweep da Fig. 11.
 */
struct Figure11Sample {
    std::string ratio_id;
    std::string ratio_label;

    std::string curve_id;
    std::string curve_label;

    int sample_index = 0;
    double c_over_a = 0.0;
    CouplerPointResult point;
};

/**
 * @brief Resultado completo da reprodução da Fig. 11.
 */
struct Figure11Result {
    Figure11Config config;
    std::string status;
    std::vector<Figure11CurveSummary> curves;
    std::vector<Figure11Sample> samples;
};

/**
 * @brief Faz o parsing de uma especificação de curva da Fig. 11.
 *
 * @details
 * O formato mais simples aceito é apenas o valor numérico de `a / A_5`.
 *
 * @throws std::invalid_argument Se o texto não puder ser interpretado.
 */
Figure11CurveSpec ParseFigure11CurveSpec(const std::string& value_text);

/**
 * @brief Faz o parsing de uma especificação de razão de índices da Fig. 11.
 *
 * @details
 * O formato mais simples aceito é o valor numérico de `n1 / n5`.
 *
 * @throws std::invalid_argument Se o texto não puder ser interpretado.
 */
Figure11IndexRatioSpec ParseFigure11IndexRatioSpec(const std::string& value_text);

/**
 * @brief Resolve a reprodução da Fig. 11.
 *
 * @throws std::invalid_argument Se a configuração for inválida.
 * @throws std::runtime_error Se houver falha numérica no solver do acoplador.
 */
Figure11Result SolveFigure11(const Figure11Config& config);

}  // namespace marcatili
