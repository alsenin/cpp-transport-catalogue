#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace string_utils {

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string);

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim);

} // namespace string_utils 