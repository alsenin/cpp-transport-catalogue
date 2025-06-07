#include "input_reader.h"
#include "string_utils.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <string>
#include <vector>
#include <regex>


using namespace geo;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return string_utils::Split(route, '>');
    }

    auto stops = string_utils::Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

struct DistanceInfo {
    std::string stop_name;
    int distance;
};

std::vector<DistanceInfo> ParseDistances(const std::string& text) {
    std::vector<DistanceInfo> result;
    std::regex pattern(R"((\d+)m to ([^,]+))");
    
    auto begin = std::sregex_iterator(text.begin(), text.end(), pattern);
    auto end = std::sregex_iterator();
    
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        DistanceInfo info;
        info.distance = std::stoi(match[1].str());
        info.stop_name = match[2].str();
        result.push_back(info);
    }
    
    return result;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            catalogue.AddStop(command.id, ParseCoordinates(command.description));
        }
    }
    
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            std::vector<DistanceInfo> distances = ParseDistances(command.description);
            std::string stop1 = command.id;
            for (const auto& distance : distances) {
                catalogue.AddDistancesBetweenStops(stop1, distance.stop_name, distance.distance);
            }
        }
    }

    for (const auto& command : commands_) {
        if (command.command == "Bus") {
            catalogue.AddRoute(command.id, ParseRoute(command.description));
        }
    }
}

void InputReader::ReadDataBase(TransportCatalogue& catalogue, std::istream& input) {
    int base_request_count;
    input >> base_request_count >> std::ws;

    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        getline(input, line);
        ParseLine(line);
    }
    ApplyCommands(catalogue);
}