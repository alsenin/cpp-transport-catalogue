#include "transport_catalogue.h"
#include "geo.h"

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#endif
#define DEBUG_PRINT(x) \
    do                 \
    {                  \
    } while (0)

#include <algorithm>
#include <unordered_set>
#include <iostream>

namespace transport_catalogue
{

    void TransportCatalogue::InvalidateCache() const
    {
        cache_valid_ = false;
        DEBUG_PRINT("Invalidating cache");
    }

    void TransportCatalogue::UpdateCache() const
    {
        if (cache_valid_)
        {
            DEBUG_PRINT("Cache is valid, skipping update");
            return;
        }

        DEBUG_PRINT("Updating cache...");
        stop_to_routes_cache_.clear();

        auto all_routes = route_container_.GetAllRoutes();
        DEBUG_PRINT("Processing " << all_routes.size() << " routes for cache");

        for (const auto &route : all_routes)
        {
            DEBUG_PRINT("Processing route '" << route->name << "' with " << route->stops.size() << " stops");
            for (const auto &stop : route->stops)
            {
                stop_to_routes_cache_[stop->name].push_back(route->name);
                DEBUG_PRINT("Added route '" << route->name << "' to stop '" << stop->name << "'");
            }
        }

        // Удаляем дубликаты маршрутов для каждой остановки
        for (auto &[stop_name, routes] : stop_to_routes_cache_)
        {
            auto original_size = routes.size();
            std::sort(routes.begin(), routes.end());
            routes.erase(std::unique(routes.begin(), routes.end()), routes.end());
            if (original_size != routes.size())
            {
                DEBUG_PRINT("Removed " << (original_size - routes.size()) << " duplicates for stop '" << stop_name << "'");
            }
        }

        DEBUG_PRINT("Cache updated with " << stop_to_routes_cache_.size() << " stops");
        cache_valid_ = true;
    }

    double TransportCatalogue::GetDistance(const std::string &from, const std::string &to) const
    {
        DEBUG_PRINT("GetDistance: " << from << " -> " << to);

        auto from_it = distances_.find(from);
        if (from_it != distances_.end())
        {
            auto to_it = from_it->second.find(to);
            if (to_it != from_it->second.end())
            {
                DEBUG_PRINT("Found exact distance: " << to_it->second << "m");
                return to_it->second;
            }
        }

        // Если точное расстояние не найдено, проверяем обратное направление
        auto to_it = distances_.find(to);
        if (to_it != distances_.end())
        {
            auto from_it_reverse = to_it->second.find(from);
            if (from_it_reverse != to_it->second.end())
            {
                DEBUG_PRINT("Found reverse distance: " << from_it_reverse->second << "m");
                return from_it_reverse->second;
            }
        }

        // Если обратное расстояние тоже не найдено, используем географическое расстояние
        auto from_stop = stop_container_.GetStop(from);
        auto to_stop = stop_container_.GetStop(to);

        if (from_stop && to_stop)
        {
            double geo_distance = geo::ComputeDistance(from_stop->coordinates, to_stop->coordinates);
            DEBUG_PRINT("Using geographic distance: " << geo_distance << "m");
            return geo_distance;
        }

        DEBUG_PRINT("Cannot calculate distance: stops not found");
        return 0.0;
    }

    void TransportCatalogue::AddStops(const std::vector<std::pair<std::string, std::pair<double, double>>> &stops)
    {
        DEBUG_PRINT("AddStops: adding " << stops.size() << " stops");
        for (const auto &[name, coords] : stops)
        {
            DEBUG_PRINT("Adding stop: " << name << " (" << coords.first << ", " << coords.second << ")");
            stop_container_.AddStop(name, coords.first, coords.second);
        }
        InvalidateCache();
    }

    void TransportCatalogue::AddRoute(const std::string &name, const std::vector<std::string> &stops, bool is_roundtrip)
    {
        DEBUG_PRINT("AddRoute: " << name << " with " << stops.size() << " stops, roundtrip: " << is_roundtrip);
        route_container_.AddRoute(name, stops, is_roundtrip);
        InvalidateCache();
    }

    void TransportCatalogue::AddDistances(const std::vector<std::tuple<std::string, std::string, double>> &distances)
    {
        DEBUG_PRINT("AddDistances: adding " << distances.size() << " distances");
        for (const auto &[from, to, distance] : distances)
        {
            DEBUG_PRINT("Adding distance: " << from << " -> " << to << " = " << distance << "m");
            distances_[from][to] = distance;
        }
    }

    std::vector<std::string> TransportCatalogue::GetStopInfo(const std::string &stop_name) const
    {
        DEBUG_PRINT("GetStopInfo: " << stop_name);
        UpdateCache();

        auto it = stop_to_routes_cache_.find(stop_name);
        if (it != stop_to_routes_cache_.end())
        {
            DEBUG_PRINT("Found " << it->second.size() << " routes for stop '" << stop_name << "'");
            return it->second;
        }

        DEBUG_PRINT("No routes found for stop '" << stop_name << "'");
        return {};
    }

    const Stop *TransportCatalogue::GetStopByName(const std::string &stop_name) const
    {
        DEBUG_PRINT("GetStopByName (coordinates): " << stop_name);
        return stop_container_.GetStop(stop_name);
    }

    RouteInfo TransportCatalogue::GetRouteInfo(const std::string &route_name) const
    {
        DEBUG_PRINT("GetRouteInfo: " << route_name);

        auto route = route_container_.GetRoute(route_name);
        if (!route)
        {
            DEBUG_PRINT("Route not found: " << route_name);
            return {0, 0, 0.0, 0.0};
        }

        RouteInfo info;
        info.stops_count = route->stops.size();
        DEBUG_PRINT("Total stops: " << info.stops_count);

        // Подсчитываем уникальные остановки
        std::unordered_set<std::string> unique_stops;
        for (const auto &stop : route->stops)
        {
            unique_stops.insert(stop->name);
        }
        info.unique_stops_count = unique_stops.size();
        DEBUG_PRINT("Unique stops: " << info.unique_stops_count);

        // Вычисляем длину маршрута
        if (route->stops.size() >= 2)
        {
            DEBUG_PRINT("Calculating route length with " << (route->stops.size() - 1) << " segments");
            double total_length = 0.0;
            for (size_t i = 0; i < route->stops.size() - 1; ++i)
            {
                double segment_length = GetDistance(route->stops[i]->name, route->stops[i + 1]->name);
                total_length += segment_length;
                DEBUG_PRINT("Segment " << i << ": " << route->stops[i]->name
                                       << " -> " << route->stops[i + 1]->name << " = " << segment_length << "m");
            }
            info.route_length = total_length;
            DEBUG_PRINT("Total route length: " << total_length << "m");

            // Вычисляем кривизну маршрута
            double straight_length;
            if (route->stops.front()->name == route->stops.back()->name)
            {
                // Для кольцевого маршрута используем длину по периметру многоугольника
                straight_length = 0.0;
                for (size_t i = 0; i < route->stops.size() - 1; ++i)
                {
                    straight_length += geo::ComputeDistance(route->stops[i]->coordinates, route->stops[i + 1]->coordinates);
                }
            }
            else
            {
                // Для линейного маршрута используем прямую линию между начальной и конечной точками
                straight_length = geo::ComputeDistance(route->stops.front()->coordinates, route->stops.back()->coordinates);
            }

            info.curvature = (straight_length > 0) ? total_length / straight_length : 1.0;
            DEBUG_PRINT("Straight distance: " << straight_length << "m, curvature: " << info.curvature);
        }
        else
        {
            DEBUG_PRINT("Route has less than 2 stops, cannot calculate length and curvature");
            info.route_length = 0.0;
            info.curvature = 1.0;
        }

        return info;
    }

    bool TransportCatalogue::RouteExists(const std::string &route_name) const
    {
        DEBUG_PRINT("RouteExists: " << route_name);
        return route_container_.Exists(route_name);
    }

} // namespace transport_catalogue
