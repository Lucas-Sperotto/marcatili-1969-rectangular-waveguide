#pragma once

#include <string>

#include "marcatili/physics/fig11.hpp"

namespace marcatili::io {

/**
 * @brief Lê a configuração da reprodução da Fig. 11 a partir de um JSON textual.
 *
 * @param json_text Conteúdo textual do arquivo JSON de entrada.
 * @param cli_output_json Caminho do JSON de saída informado na linha de comando.
 *
 * @return Configuração preenchida para a reprodução da Fig. 11.
 *
 * @details
 * Se `csv_file` não for informado no JSON, um caminho padrão é derivado a partir
 * do arquivo JSON de saída informado em `cli_output_json`.
 */
marcatili::Figure11Config ParseFigure11Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

/**
 * @brief Constrói o relatório JSON de saída da reprodução da Fig. 11.
 */
std::string BuildFigure11JsonReport(
    const marcatili::Figure11Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

/**
 * @brief Constrói o relatório CSV de saída da reprodução da Fig. 11.
 */
std::string BuildFigure11CsvReport(const marcatili::Figure11Result& result);

}  // namespace marcatili::io
