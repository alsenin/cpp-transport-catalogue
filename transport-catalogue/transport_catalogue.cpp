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

void TransportCatalogue::AddDistancesBetweenStops(const std::string& stop1, const std::string& stop2, double distance) {
    distances_between_stops_[std::make_pair(stop1, stop2)] = distance;
 // distances_between_stops_[std::make_pair(stop2, stop1)] = distance;
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

    double geo_length = GetRouteGeoLength(name);
    double route_length = GetRouteLength(name);
    double curvature = route_length / geo_length;
    
    return RouteInfo{static_cast<int>(it->second->stops.size()),
                     static_cast<int>(std::set<const Stop*>(it->second->stops.begin(), it->second->stops.end()).size()),
                     route_length,
                     curvature};
}

void TransportCatalogue::PrintRoutes(std::ostream& out) const {
    std::cout << "--------------------------------" << std::endl;
    for (const auto& route : routes_) {
        out << "Route: " << route.name << std::endl << "\tStops: ";
        for (const auto& stop : route.stops) {
            out << stop->name << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "--------------------------------" << std::endl;
}

void TransportCatalogue::PrintDistances(std::ostream& out) const {
    for (const auto& [stops, distance] : distances_between_stops_) {
        out << "From " << stops.first << " to " << stops.second << ": " << distance << "m\n";
    }
    std::cout << "--------------------------------" << std::endl;
}

void TransportCatalogue::PrintStops(std::ostream& out) const {
    for (const auto& stop : stops_) {
        out << "Stop: " << stop.name << std::endl;
        out << "\tCoordinates: " << stop.coordinates.lat << ", " << stop.coordinates.lng << std::endl;
        out << "\tRoutes: ";
        auto it = routes_by_stop_.find(stop.name);
        if (it != routes_by_stop_.end()) {
            for (const auto& route : it->second) {
                out << route << " ";
            }
        }
        out << std::endl;
    }
    std::cout << "--------------------------------" << std::endl;
}

void TransportCatalogue::PrintRoutesByStop(std::ostream& out) const {
    for (const auto& [stop, routes] : routes_by_stop_) {
        out << "Stop: " << stop << std::endl;
        out << "\tRoutes: ";
        for (const auto& route : routes) {
            out << route << " ";
        }
        out << std::endl;
    }
    std::cout << "--------------------------------" << std::endl;
}

double TransportCatalogue::GetRouteLength(const std::string& name) const {
    const Route* route = route_by_name_.at(name);
    double length = 0;
    for (size_t i = 0; i < route->stops.size() - 1; ++i) {
        auto it = distances_between_stops_.find(std::make_pair(route->stops[i]->name, route->stops[i + 1]->name));
        if (it != distances_between_stops_.end()) {
            length += it->second;
        } else {
            // Если расстояние в одном направлении не найдено, пробуем в обратном
            it = distances_between_stops_.find(std::make_pair(route->stops[i + 1]->name, route->stops[i]->name));
            if (it != distances_between_stops_.end()) {
                length += it->second;
            }
        }
    }
    return length;
}

double TransportCatalogue::GetRouteGeoLength(const std::string& name) const {
    const Route* route = route_by_name_.at(name);
    double length = 0;
    for (size_t i = 0; i < route->stops.size() - 1; ++i) {
        length += ComputeDistance(route->stops[i]->coordinates, route->stops[i + 1]->coordinates);
    }
    return length;
}