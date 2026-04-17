#pragma once

#include <string>
#include <vector>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili {

/**
 * @brief Parâmetros da busca numérica de cutoff usada na reprodução da Tabela I.
 */
struct Table1SearchConfig {
    int max_p = 4;
    int max_q = 4;
    double b_normalized_min = 0.01;
    double b_normalized_max = 60.0;
    double cutoff_tolerance = 1e-6;
};

/**
 * @brief Especificação de uma linha da Tabela I.
 *
 * @details
 * Cada linha representa uma geometria/material do artigo, para a qual o código
 * procura o primeiro cutoff de modo superior que limita a operação monomodo.
 */
struct Table1RowSpec {
    std::string row_id;
    std::string article_panel_id;

    double a_over_b = 0.0;

    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;

    double article_dimension_normalized = 0.0;
};

/**
 * @brief Configuração completa da reprodução da Tabela I.
 */
struct Table1Config {
    std::string case_id;
    std::string article_target;

    std::string summary_csv_output_path;
    std::string details_csv_output_path;

    // Interpretação explícita da dimensão tabulada do artigo.
    // Valor suportado atualmente: "a_times_n1_over_lambda".
    std::string table_entry_interpretation = "a_times_n1_over_lambda";

    double wavelength = 0.0;
    double n1 = 0.0;

    Table1SearchConfig search;
    std::vector<SingleGuideSolverModel> solver_models;
    std::vector<Table1RowSpec> rows;
};

/**
 * @brief Cutoff estimado para um modo candidato em uma linha da Tabela I.
 */
struct Table1ModeCutoff {
    std::string row_id;
    std::string article_panel_id;

    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kExact;
    SingleGuideFamily family = SingleGuideFamily::kEy;

    int p = 1;
    int q = 1;

    std::string mode_id;

    bool cutoff_found = false;
    bool guided_at_search_min = false;
    bool guided_at_search_max = false;
    bool guided_in_search_window = false;
    std::string cutoff_status;

    double cutoff_b_normalized = 0.0;
    double cutoff_a_normalized = 0.0;
    double cutoff_b_over_A4 = 0.0;
};

/**
 * @brief Resumo final de uma linha da Tabela I.
 */
struct Table1RowSummary {
    std::string row_id;
    std::string article_panel_id;

    SingleGuideSolverModel solver_model = SingleGuideSolverModel::kExact;

    double a_over_b = 0.0;
    double n2 = 0.0;
    double n3 = 0.0;
    double n4 = 0.0;
    double n5 = 0.0;

    bool limiting_cutoff_found = false;
    std::string limiting_cutoff_status;
    std::string limiting_mode_id;
    SingleGuideFamily limiting_family = SingleGuideFamily::kEy;
    int limiting_p = 0;
    int limiting_q = 0;

    double article_dimension_normalized = 0.0;
    double computed_dimension_normalized = 0.0;

    double computed_b_normalized = 0.0;
    double computed_a_normalized = 0.0;
    double computed_b_over_A4 = 0.0;

    double absolute_error = 0.0;
    double relative_error = 0.0;

    bool ex11_guided_just_below_cutoff = false;
    bool ey11_guided_just_below_cutoff = false;
};

/**
 * @brief Resultado completo da reprodução da Tabela I.
 */
struct Table1Result {
    Table1Config config;
    std::string status;
    std::vector<Table1RowSummary> row_summaries;
    std::vector<Table1ModeCutoff> mode_cutoffs;
};

/**
 * @brief Constrói o identificador textual de um modo.
 */
std::string BuildTable1ModeId(SingleGuideFamily family, int p, int q);

/**
 * @brief Resolve a reprodução da Tabela I.
 *
 * @details
 * Para cada linha do artigo, o código busca o primeiro cutoff de modo superior
 * entre as famílias `E_y` e `E_x`, excluindo os modos fundamentais 11. O menor
 * cutoff encontrado define o limite monomodo comparado à dimensão tabulada.
 */
Table1Result SolveTable1(const Table1Config& config);

}  // namespace marcatili
