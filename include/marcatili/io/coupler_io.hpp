#pragma once

#include <string>

#include "marcatili/physics/coupler.hpp"

namespace marcatili::io {

/**
 * @brief Lê a configuração de um ponto do acoplador a partir de um JSON textual.
 *
 * @param json_text Conteúdo textual do arquivo JSON de entrada.
 * @param cli_output_json Caminho do JSON de saída informado na linha de comando.
 *
 * @return Configuração preenchida para o solver do acoplador.
 *
 * @details
 * Esta rotina aceita algumas chaves alternativas, por exemplo:
 * - `normalized_geometry.a_over_A5` ou `a_over_A5`
 * - `normalized_geometry.c_over_a` ou `c_over_a`
 * - `materials.index_ratio_squared` ou `index_ratio_squared`
 *
 * Se `csv_file` não for informado no JSON, um caminho padrão é derivado a partir
 * do arquivo JSON de saída informado em `cli_output_json`.
 */
marcatili::CouplerPointConfig ParseCouplerPointConfig(
    const std::string& json_text,
    const std::string& cli_output_json
);

/**
 * @brief Constrói o relatório JSON de saída do solver do acoplador.
 */
std::string BuildCouplerPointJsonReport(
    const marcatili::CouplerPointResult& result,
    const std::string& input_file,
    const std::string& output_json_file
);

/**
 * @brief Constrói o relatório CSV de saída do solver do acoplador.
 */
std::string BuildCouplerPointCsvReport(const marcatili::CouplerPointResult& result);

}  // namespace marcatili::io
