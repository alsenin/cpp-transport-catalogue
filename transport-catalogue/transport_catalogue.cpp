#include "transport_catalogue.h"
#include "geo.h"
#include <set>
#include <iostream>
#include <optional>

void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) {
    stops_.push_back({name, coordinates});
    stop_by_name_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddRoute(const std::string& name, const std::vector<std::string_view>& stops) {
    std::vector<const Stop*> stops_pointers;
    stops_pointers.reserve(stops.size());
    
    for (const auto& stop_name : stops) {
        auto it = stop_by_name_.find(std::string(stop_name));
        if (it == stop_by_name_.end()) {
            continue;
        }
        stops_pointers.push_back(it->second);
        routes_by_stop_[it->second->name].insert(name);
    }
    
    if (stops_pointers.empty()) {
        return;
    }
    
    routes_.push_back({name, std::move(stops_pointers)});
    route_by_name_[routes_.back().name] = &routes_.back();
}

std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(const std::string& name) const {
    
    if (stop_by_name_.find(name) == stop_by_name_.end()) {
        return std::nullopt;
    }
    if (routes_by_stop_.find(name) == routes_by_stop_.end()) {
        return std::set<std::string>();
    }
    return routes_by_stop_.at(name);
}

std::optional<RouteInfo> TransportCatalogue::GetRouteInfo(const std::string& name) const {
    auto it = route_by_name_.find(name);
    if (it == route_by_name_.end()) {
        return std::nullopt;
    }
    return RouteInfo{static_cast<int>(it->second->stops.size()),
                     static_cast<int>(std::set<const Stop*>(it->second->stops.begin(), it->second->stops.end()).size()),
                     GetRouteLength(name)};
}
