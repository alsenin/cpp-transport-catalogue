#include "transport_catalogue.h"
#include "geo.h"
#include <set>
#include <iostream>

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

void TransportCatalogue::PrintRouteInfo(const std::string& name, std::ostream& output) const {
    auto it = route_by_name_.find(name);
    if (it == route_by_name_.end()) {
        output << "Bus " << name << ": not found" << std::endl;
        return;
    }

    output << "Bus " << name << ": "
           << GetStopsCount(name) << " stops on route, "
           << GetUniqueStopsCount(name) << " unique stops, "
           << GetRouteLength(name) << " route length" << std::endl;
}

void TransportCatalogue::PrintStopInfo(const std::string& name, std::ostream& output) const {
    auto it = stop_by_name_.find(name);
    if (it == stop_by_name_.end()) {
        output << "Stop " << name << ": not found" << std::endl;
        return;
    }

    output << "Stop " << name << ": ";
    
    auto routes_it = routes_by_stop_.find(name);
    if (routes_it == routes_by_stop_.end() || routes_it->second.empty()) {
        output << "no buses" << std::endl;
    } else {
        output << "buses ";
        bool first = true;
        for (const auto& route_name : routes_it->second) {
            if (!first) {
                output << " ";
            }
            output << route_name;
            first = false;
        }
        output << std::endl;
    }
}
