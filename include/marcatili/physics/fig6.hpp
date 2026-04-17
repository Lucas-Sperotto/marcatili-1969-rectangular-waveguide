#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Modelo geométrico usado na reprodução da Fig. 6.
 *
 * @details
 * - `kRectangular`: guia retangular geral;
 * - `kSlab`: limite de lâmina.
 */
enum class Figure6GeometryModel {
    kRectangular,
    kSlab
};

/**
 * @brief Especificação modal de uma curva da Fig. 6.
 */
struct Figure6ModeSpec {
    SingleGuideFamily family = SingleGuideFamily::kEy;
    int p = 1;
    int q = 1;
    std::string curve_id;
};

/**
 * @brief Variante material para subcasos de um mesmo painel.
 *
 * @details
 * Permite reproduzir painéis em que os índices externos variam entre curvas.
 */
struct Figure6MaterialVariant {
    std::string variant_id;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;
};

/**
 * @brief Configuração completa da reprodução de um painel da Fig. 6.
 */
struct Figure6Config {
    std::string case_id;
    std::string article_target;
    std::string panel_id;
    std::string csv_output_path;

    Figure6GeometryModel geometry_model = Figure6GeometryModel::kRectangular;

    double wavelength = 0.0;
    double a_over_b = 0.0;

    double n1 = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;

    double b_over_A4_min = 0.0;
    double b_over_A4_max = 0.0;
    int point_count = 0;

    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Figure6ModeSpec> modes;
    std::vector<Figure6MaterialVariant> material_variants;
};

/**
 * @brief Resumo por curva gerada no sweep da Fig. 6.
 */
struct Figure6CurveSummary {
    std::string variant_id;
    Figure6ModeSpec mode;
    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kClosedForm;
    int total_points = 0;
    int guided_points = 0;
    int valid_points = 0;
};

/**
 * @brief Amostra individual produzida ao longo do sweep.
 */
struct Figure6Sample {
    std::string panel_id;
    std::string variant_id;
    std::string curve_id;
    int sample_index = 0;
    double b_over_A4 = 0.0;
    SingleGuideResult point;
};

/**
 * @brief Resultado completo da reprodução de um painel da Fig. 6.
 */
struct Figure6Result {
    Figure6Config config;
    std::string status;
    std::vector<Figure6CurveSummary> curves;
    std::vector<Figure6Sample> samples;
};

std::string ToString(Figure6GeometryModel geometry_model);
Figure6GeometryModel ParseFigure6GeometryModel(const std::string& geometry_model_text);
Figure6ModeSpec ParseFigure6ModeSpec(const std::string& mode_text);
Figure6Result SolveFigure6(const Figure6Config& config);

}  // namespace marcatili
