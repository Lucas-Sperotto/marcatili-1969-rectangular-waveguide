#pragma once

#include <string>

namespace marcatili::io {

/**
 * @brief Escapa texto para uso seguro em strings JSON.
 */
std::string EscapeJson(const std::string& text);

/**
 * @brief Escapa texto para uso seguro em um campo CSV.
 */
std::string EscapeCsv(const std::string& text);

/**
 * @brief Lê um arquivo texto inteiro para uma string.
 *
 * @throws std::runtime_error Se o arquivo não puder ser aberto.
 */
std::string ReadTextFile(const std::string& path);

/**
 * @brief Escreve uma string inteira em um arquivo texto.
 *
 * @details
 * Se o diretório pai não existir, ele é criado automaticamente.
 *
 * @throws std::runtime_error Se o arquivo não puder ser aberto ou escrito.
 */
void WriteTextFile(const std::string& path, const std::string& content);

/**
 * @brief Substitui a extensão de um caminho por outra.
 */
std::string ReplaceExtension(const std::string& path, const std::string& new_extension);

}  // namespace marcatili::io
