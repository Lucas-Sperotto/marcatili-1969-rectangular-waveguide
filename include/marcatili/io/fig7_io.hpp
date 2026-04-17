#pragma once

#include <string>

#include "marcatili/physics/fig7.hpp"

namespace marcatili::io {

/**
 * @brief Lê a configuração da reprodução da Fig. 7 a partir de um JSON textual.
 *
 * @param json_text Conteúdo textual do arquivo JSON de entrada.
 * @param cli_output_json Caminho do JSON de saída informado na linha de comando.
 *
 * @return Configuração preenchida para a reprodução da Fig. 7.
 *
 * @details
 * Se os caminhos dos CSVs não forem informados no JSON, eles são derivados
 * automaticamente a partir do arquivo JSON de saída informado em `cli_output_json`.
 */
marcatili::Figure7Config ParseFigure7Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

/**
 * @brief Constrói o relatório JSON de saída da reprodução da Fig. 7.
 */
std::string BuildFigure7JsonReport(
    const marcatili::Figure7Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

/**
 * @brief Constrói o CSV com as amostras das linhas do nomograma.
 */
std::string BuildFigure7LinesCsvReport(const marcatili::Figure7Result& result);

/**
 * @brief Constrói o CSV com as interseções entre linhas modais e linhas de C.
 */
std::string BuildFigure7IntersectionsCsvReport(const marcatili::Figure7Result& result);

}  // namespace marcatili::io
