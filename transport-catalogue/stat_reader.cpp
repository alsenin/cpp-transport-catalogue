#include "stat_reader.h"
#include <iostream>
#include <iomanip>

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
            auto route_info = transport_catalogue.GetRouteInfo(bus_name);
            if (route_info) {
                output << "Bus " << bus_name << ": " 
                       << route_info->stops_count << " stops on route, " 
                       << route_info->unique_stops_count << " unique stops, " 
                       << std::setprecision(6) << route_info->route_length << " route length"
                       << ", " << std::setprecision(6) << route_info->curvature << " curvature"
                       << std::endl;
            } else {
                output << "Bus " << bus_name << ": not found" << std::endl;
            }
        }
    } else if (trimmed.substr(0, 4) == "Stop") {
        std::string stop_name = ExtractName(trimmed, 4);
        if (!stop_name.empty()) {
            output << "Stop " << stop_name << ": ";
            auto stop_info = transport_catalogue.GetStopInfo(stop_name);
            if (stop_info) {
                if (stop_info->size() > 0) {
                    output << "buses";
                    for (const auto& bus : *stop_info) {
                        output << " " << bus;
                    }
                } else {
                    output << "no buses";
                }
            } else {
                output << "not found";
            }
            output << std::endl;
        }
    }
}

void DoStatRequests(TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;

    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}