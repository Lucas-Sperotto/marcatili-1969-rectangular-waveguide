#pragma once

#include <string>

#include "marcatili/physics/table1.hpp"

namespace marcatili::io {

/**
 * @brief Lê a configuração da reprodução da Tabela I a partir de um JSON textual.
 *
 * @param json_text Conteúdo textual do arquivo JSON de entrada.
 * @param cli_output_json Caminho do JSON de saída informado na linha de comando.
 *
 * @return Configuração preenchida para a reprodução da Tabela I.
 *
 * @details
 * Se os caminhos dos CSVs não forem informados no JSON, eles são derivados
 * automaticamente a partir do arquivo JSON de saída informado em `cli_output_json`.
 */
marcatili::Table1Config ParseTable1Config(
    const std::string& json_text,
    const std::string& cli_output_json
);

/**
 * @brief Constrói o relatório JSON de saída da reprodução da Tabela I.
 */
std::string BuildTable1JsonReport(
    const marcatili::Table1Result& result,
    const std::string& input_file,
    const std::string& output_json_file
);

/**
 * @brief Constrói o CSV-resumo da reprodução da Tabela I.
 */
std::string BuildTable1SummaryCsvReport(const marcatili::Table1Result& result);

/**
 * @brief Constrói o CSV detalhado com os cutoffs modais da Tabela I.
 */
std::string BuildTable1DetailsCsvReport(const marcatili::Table1Result& result);

}  // namespace marcatili::io
