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

struct RouteInfo {
    int stops_count;
    int unique_stops_count;
    double route_length;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    void AddStop(const std::string& name, const geo::Coordinates& coordinates);
    void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
    std::optional<std::set<std::string>> GetStopInfo(const std::string& name) const;
    std::optional<RouteInfo> GetRouteInfo(const std::string& name) const;

private:
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
