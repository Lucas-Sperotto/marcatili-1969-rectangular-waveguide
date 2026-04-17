#pragma once

#include <optional>
#include <string>
#include <vector>

namespace marcatili::io {

/**
 * @brief Procura uma string em um JSON controlado pelo schema do repositório.
 *
 * @details
 * Esta camada suporta apenas o subconjunto de JSON necessário aos arquivos
 * de entrada do projeto. Não é um parser JSON geral.
 *
 * Chaves podem ser fornecidas como caminhos com ponto, por exemplo:
 * `geometry.a`
 */
std::optional<std::string> FindStringValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Procura um número real em um JSON controlado pelo schema do repositório.
 */
std::optional<double> FindDoubleValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Procura um inteiro em um JSON controlado pelo schema do repositório.
 */
std::optional<int> FindIntValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Procura um array de strings em um JSON controlado pelo schema do repositório.
 *
 * @details
 * Se a chave estiver ausente ou o valor não puder ser interpretado como array
 * de strings, retorna vetor vazio.
 */
std::vector<std::string> FindStringArrayValues(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Retorna o valor bruto JSON associado a uma chave.
 *
 * @details
 * O valor retornado preserva sua forma textual JSON, por exemplo:
 * `"abc"`, `1.5`, `{...}`, `[...]`.
 */
std::optional<std::string> FindRawJsonValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Procura um array de objetos JSON e devolve cada objeto como texto bruto.
 *
 * @details
 * Se a chave estiver ausente ou o valor não puder ser interpretado como array
 * de objetos, retorna vetor vazio.
 */
std::vector<std::string> FindObjectArrayValues(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Exige uma string válida em uma chave obrigatória.
 *
 * @throws std::runtime_error Se a chave estiver ausente ou o valor for inválido.
 */
std::string RequireStringValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Exige um número real válido em uma chave obrigatória.
 *
 * @throws std::runtime_error Se a chave estiver ausente ou o valor for inválido.
 */
double RequireDoubleValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Exige um inteiro válido em uma chave obrigatória.
 *
 * @throws std::runtime_error Se a chave estiver ausente ou o valor for inválido.
 */
int RequireIntValue(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Exige um array de strings em uma chave obrigatória.
 *
 * @details
 * Diferentemente da versão anterior, um array vazio `[]` é aceito como valor
 * presente e válido. O erro é lançado apenas se a chave estiver ausente ou se
 * o valor não for um array de strings válido.
 *
 * @throws std::runtime_error Se a chave estiver ausente ou o valor for inválido.
 */
std::vector<std::string> RequireStringArrayValues(
    const std::string& json_text,
    const std::string& key
);

/**
 * @brief Exige um array de objetos em uma chave obrigatória.
 *
 * @details
 * Um array vazio `[]` é aceito como valor presente e válido.
 *
 * @throws std::runtime_error Se a chave estiver ausente ou o valor for inválido.
 */
std::vector<std::string> RequireObjectArrayValues(
    const std::string& json_text,
    const std::string& key
);

}  // namespace marcatili::io
