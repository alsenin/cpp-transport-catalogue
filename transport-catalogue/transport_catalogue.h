#pragma once

#include "domain.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace transport_catalogue {

struct RouteInfo {
    int stops_count;
    int unique_stops_count;
    double route_length;
    double curvature;
};

class TransportCatalogue {
public:
    TransportCatalogue() : route_container_(&stop_container_) {}
    
    // Добавление остановок из списка инициализации
    void AddStops(const std::vector<std::pair<std::string, std::pair<double, double>>>& stops);
    
    // Добавление одного маршрута
    void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip = false);
    
    // Добавление расстояний между остановками
    void AddDistances(const std::vector<std::tuple<std::string, std::string, double>>& distances);
    
    // Получение информации об остановке
    std::vector<std::string> GetStopInfo(const std::string& stop_name) const;
    
    // Получение информации об остановке (координаты)
    const Stop* GetStopByName(const std::string& stop_name) const;
    
    // Получение информации о маршруте
    RouteInfo GetRouteInfo(const std::string& route_name) const;
    
    // Дополнительные методы для доступа к контейнерам
    const domain::StopContainer& GetStopContainer() const { return stop_container_; }
    const domain::RouteContainer& GetRouteContainer() const { return route_container_; }
    
    // Проверка существования
    bool RouteExists(const std::string& route_name) const;

    // Получение реального расстояния между остановками
    double GetDistance(const std::string& from, const std::string& to) const;

private:
    // Вспомогательные методы
    void InvalidateCache() const;
    void UpdateCache() const;
    
private:
    domain::StopContainer stop_container_;
    domain::RouteContainer route_container_;
    
    // Кэш для быстрого поиска маршрутов через остановку
    mutable std::unordered_map<std::string, std::vector<std::string>> stop_to_routes_cache_;
    mutable bool cache_valid_ = true;
    
    // Расстояния между остановками
    std::unordered_map<std::string, std::unordered_map<std::string, double>> distances_;
};

} // namespace transport_catalogue