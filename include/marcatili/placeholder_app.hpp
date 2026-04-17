#pragma once

#include <string>

namespace marcatili {

/**
 * @brief Especificação mínima de um executável placeholder.
 *
 * @details
 * Esta estrutura mantém explícito, já na fase inicial do repositório,
 * qual executável está sendo invocado e qual artefato do artigo ele deverá
 * reproduzir futuramente.
 */
struct PlaceholderAppSpec {
    std::string executable_name;
    std::string objective;
};

/**
 * @brief Executa o fluxo padrão de um aplicativo placeholder.
 *
 * @param spec Metadados do executável.
 * @param argc Quantidade de argumentos recebidos pela linha de comando.
 * @param argv Vetor de argumentos da linha de comando.
 *
 * @return Código de saída do processo.
 *
 * @details
 * Todo placeholder aceita:
 *
 *   <input_file> [output_json]
 *
 * Se `output_json` for fornecido, o relatório placeholder é escrito em arquivo.
 * Caso contrário, o JSON é emitido em `stdout`.
 */
int RunPlaceholderApp(const PlaceholderAppSpec& spec, int argc, char** argv);

}  // namespace marcatili
