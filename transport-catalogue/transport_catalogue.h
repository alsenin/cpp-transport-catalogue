#pragma once

#include "geo.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <deque>
#include <ostream>
#include <set>

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Route {
    std::string name;
    std::vector<const Stop*> stops;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    void AddStop(const std::string& name, const geo::Coordinates& coordinates);
    void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
    void PrintRouteInfo(const std::string& name, std::ostream& output) const;
    void PrintStopInfo(const std::string& name, std::ostream& output) const;

private:
int GetUniqueStopsCount(const std::string& name) const {
    std::set<const Stop*> unique_stops;
    for (const auto& stop : route_by_name_.at(name)->stops) {
        unique_stops.insert(stop);
    }
    return unique_stops.size();
}

int GetStopsCount(const std::string& name) const {
    if (route_by_name_.find(name) == route_by_name_.end()) {
        return 0;
    }
    return route_by_name_.at(name)->stops.size();
}

double GetRouteLength(const std::string& name) const {
    const Route* route = route_by_name_.at(name);
    double length = 0;
    for (size_t i = 0; i < route->stops.size() - 1; ++i) {
        length += ComputeDistance(route->stops[i]->coordinates, route->stops[i + 1]->coordinates);
    }
    return length;
}

private:
    std::deque<Stop> stops_;
    std::deque<Route> routes_;
    std::unordered_map<std::string, const Stop*> stop_by_name_;
    std::unordered_map<std::string, const Route*> route_by_name_;
    std::unordered_map<std::string, std::set<std::string>> routes_by_stop_;
};
