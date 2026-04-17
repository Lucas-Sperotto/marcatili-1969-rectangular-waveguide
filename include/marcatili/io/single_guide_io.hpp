#pragma once

#include <string>

#include "marcatili/physics/single_guide.hpp"

namespace marcatili::io {

/**
 * @brief Lê a configuração do solver de guia único a partir de um JSON textual.
 *
 * @param json_text Conteúdo textual do arquivo JSON de entrada.
 * @param cli_output_json Caminho do JSON de saída informado na linha de comando.
 *
 * @return Configuração preenchida para o solver.
 *
 * @details
 * Esta rotina aceita tanto chaves planas quanto caminhos com ponto para alguns
 * campos, por exemplo:
 * - `wavelength` ou `geometry.wavelength`
 * - `p` ou `mode_indices.p`
 *
 * Se `csv_file` não for informado no JSON, um caminho padrão é derivado a partir
 * do arquivo JSON de saída informado em `cli_output_json`.
 */
marcatili::SingleGuideConfig ParseSingleGuideConfig(
    const std::string& json_text,
    const std::string& cli_output_json
);

/**
 * @brief Constrói o relatório JSON de saída do solver de guia único.
 */
std::string BuildSingleGuideJsonReport(
    const marcatili::SingleGuideResult& result,
    const std::string& input_file,
    const std::string& output_json_file
);

/**
 * @brief Constrói o relatório CSV de saída do solver de guia único.
 */
std::string BuildSingleGuideCsvReport(const marcatili::SingleGuideResult& result);

}  // namespace marcatili::io
