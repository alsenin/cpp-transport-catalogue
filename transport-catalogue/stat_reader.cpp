#include "stat_reader.h"
#include <iostream>

std::string ExtractName(std::string_view trimmed, size_t command_length)
{
    // Пропускаем команду и все пробелы после неё
    size_t name_start = trimmed.find_first_not_of(' ', command_length);
    if (name_start == std::string_view::npos)
    {
        return "";
    }

    // Находим последний непробельный символ
    size_t name_end = trimmed.find_last_not_of(' ');
    if (name_end == std::string_view::npos)
    {
        return "";
    }

    // Выделяем имя
    return std::string(trimmed.substr(name_start, name_end - name_start + 1));
}

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    auto trimmed = string_utils::Trim(request);
    if (trimmed.empty()) {
        return;
    }

    if (trimmed.substr(0, 3) == "Bus") {
        std::string bus_name = ExtractName(trimmed, 3);
        if (!bus_name.empty()) {
            transport_catalogue.PrintRouteInfo(bus_name, output);
        }
    } else if (trimmed.substr(0, 4) == "Stop") {
        std::string stop_name = ExtractName(trimmed, 4);
        if (!stop_name.empty()) {
            transport_catalogue.PrintStopInfo(stop_name, output);
        }
    }
}