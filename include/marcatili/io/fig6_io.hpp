#pragma once

#include <string>

#include "marcatili/physics/fig6.hpp"

namespace marcatili::io {

/**
 * @brief Lê a configuração da reprodução da Fig. 6 a partir de um JSON textual.
 *
 * @param json_text Conteúdo textual do arquivo JSON de entrada.
 * @param cli_output_json Caminho do JSON de saída informado na linha de comando.
 *
 * @return Configuração preenchida para a reprodução da Fig. 6.
 *
 * @details
 * Se `csv_file` não for informado no JSON, um caminho padrão é derivado a partir
 * do arquivo JSON de saída informado em `cli_output_json`.
 */
marcatili::Figure6Config ParseFigure6Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

/**
 * @brief Constrói o relatório JSON de saída da reprodução da Fig. 6.
 */
std::string BuildFigure6JsonReport(
    const marcatili::Figure6Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

/**
 * @brief Constrói o relatório CSV de saída da reprodução da Fig. 6.
 */
std::string BuildFigure6CsvReport(const marcatili::Figure6Result& result);

}  // namespace marcatili::io
