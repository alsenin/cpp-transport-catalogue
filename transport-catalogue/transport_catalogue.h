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
    double curvature;
};

struct PairHash {
    size_t operator()(const std::pair<std::string, std::string>& pair) const {
        return std::hash<std::string>{}(pair.first) ^ (std::hash<std::string>{}(pair.second) << 1);
    }
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    void AddStop(const std::string& name, const geo::Coordinates& coordinates);
    void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);
    void AddDistancesBetweenStops(const std::string& stop1, const std::string& stop2, double distance);
    std::optional<std::set<std::string>> GetStopInfo(const std::string& name) const;
    std::optional<RouteInfo> GetRouteInfo(const std::string& name) const;
    void PrintRoutes(std::ostream& out) const;
    void PrintDistances(std::ostream& out) const;
    void PrintStops(std::ostream& out) const;
    void PrintRoutesByStop(std::ostream& out) const;

private:
    double GetRouteGeoLength(const std::string& name) const;
    double GetRouteLength(const std::string& name) const;

private:
    std::deque<Stop> stops_;
    std::deque<Route> routes_;
    std::unordered_map<std::string, const Stop*> stop_by_name_;
    std::unordered_map<std::string, const Route*> route_by_name_;
    std::unordered_map<std::string, std::set<std::string>> routes_by_stop_;
    std::unordered_map<std::pair<std::string, std::string>, double, PairHash> distances_between_stops_;
};
