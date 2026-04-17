#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Especificação modal de uma curva da Fig. 8.
 */
struct Figure8ModeSpec {
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    std::string curve_id;
};

/**
 * @brief Configuração completa da reprodução da Fig. 8.
 *
 * @details
 * A variável horizontal da figura é `a / A`, onde `A` é calculado a partir
 * do contraste entre `n1` e `n4`. O builder da figura executa um sweep nessa
 * coordenada normalizada e delega a física ao solver do guia metalizado.
 */
struct Figure8Config {
    std::string case_id;
    std::string article_target;
    std::string ocr_note;
    std::string csv_output_path;

    double wavelength = 0.0;
    double a_over_b = 0.0;
    double n1 = 0.0;
    double n4 = 0.0;

    double a_over_A_min = 0.0;
    double a_over_A_max = 0.0;
    int point_count = 0;

    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Figure8ModeSpec> modes;
};

/**
 * @brief Resumo por curva gerada no sweep da Fig. 8.
 */
struct Figure8CurveSummary {
    Figure8ModeSpec mode;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    int total_points = 0;
    int guided_points = 0;
    int valid_points = 0;
};

/**
 * @brief Amostra individual produzida ao longo do sweep da Fig. 8.
 */
struct Figure8Sample {
    std::string curve_id;
    int sample_index = 0;
    double a_over_A = 0.0;
    SingleGuideResult point;
};

/**
 * @brief Resultado completo da reprodução da Fig. 8.
 */
struct Figure8Result {
    Figure8Config config;
    std::string status;
    std::vector<Figure8CurveSummary> curves;
    std::vector<Figure8Sample> samples;
};

/**
 * @brief Faz o parsing de uma especificação modal da Fig. 8.
 *
 * @throws std::invalid_argument Se o texto estiver em formato inválido.
 */
Figure8ModeSpec ParseFigure8ModeSpec(const std::string& mode_text);

/**
 * @brief Resolve a reprodução da Fig. 8.
 *
 * @throws std::invalid_argument Se a configuração for inválida.
 * @throws std::runtime_error Se houver falha numérica no solver subjacente.
 */
Figure8Result SolveFigure8(const Figure8Config& config);

}  // namespace marcatili
