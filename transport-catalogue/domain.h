#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include "geo.h"

#ifdef DEBUG_OUTPUT_DOMAIN
#define DEBUG_PRINT(x) std::cerr << "[DEBUG][DOMAIN] " << x << std::endl
#else
#define DEBUG_PRINT(x) do {} while(0)
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <deque>
#include <ostream>
#include <set>
#include <memory>
#include <iostream>

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Route {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip = false;
};

namespace domain {

// Базовый класс-контейнер
template<typename T>
class Container {
protected:
    std::unordered_map<std::string, std::unique_ptr<T>> items_;
    
public:
    virtual ~Container() = default;
    
    // Проверить существование элемента
    virtual bool Exists(const std::string& name) const {
        bool exists = items_.find(name) != items_.end();
        DEBUG_PRINT("Checking existence of '" << name << "': " << (exists ? "true" : "false"));
        return exists;
    }

protected:
    // Добавить элемент
    virtual void Add(std::unique_ptr<T> item) = 0;
    
    // Получить элемент по имени
    virtual const T* Get(const std::string& name) const {
        auto it = items_.find(name);
        if (it != items_.end()) {
            DEBUG_PRINT("Found item: " << name);
            return it->second.get();
        } else {
            DEBUG_PRINT("Item not found: " << name);
            return nullptr;
        }
    }
};

// Производный класс для работы с остановками
class StopContainer : public Container<Stop> {
public:
    // Добавить остановку
    void Add(std::unique_ptr<Stop> stop) override {
        if (stop && !stop->name.empty()) {
            DEBUG_PRINT("Adding stop: " << stop->name 
                      << " at (" << stop->coordinates.lat << ", " << stop->coordinates.lng << ")");
            items_[stop->name] = std::move(stop);
        } else {
            DEBUG_PRINT("Failed to add stop: invalid data");
        }
    }
    
    // Получить остановку по имени
    const Stop* GetStop(const std::string& name) const {
        DEBUG_PRINT("GetStop: " << name);
        return Get(name);
    }
    
    // Добавить остановку с координатами
    void AddStop(const std::string& name, double lat, double lng) {
        DEBUG_PRINT("AddStop: " << name << " (" << lat << ", " << lng << ")");
        auto stop = std::make_unique<Stop>();
        stop->name = name;
        stop->coordinates.lat = lat;
        stop->coordinates.lng = lng;
        Add(std::move(stop));
    }
    
    // Получить все остановки
    std::vector<const Stop*> GetAllStops() const {
        DEBUG_PRINT("GetAllStops");
        std::vector<const Stop*> result;
        result.reserve(items_.size());
        for (const auto& [name, item] : items_) {
            result.push_back(item.get());
        }
        return result;
    }
};

// Производный класс для работы с маршрутами
class RouteContainer : public Container<Route> {
public:
    explicit RouteContainer(StopContainer* stop_container) 
        : stop_container_(stop_container) {
        DEBUG_PRINT("RouteContainer created with stop_container: " << (stop_container ? "valid" : "null"));
    }
    
    // Добавить маршрут
    void Add(std::unique_ptr<Route> route) override {
        if (route && !route->name.empty()) {
            DEBUG_PRINT("Adding route: " << route->name << " with " << route->stops.size() << " stops");
            items_[route->name] = std::move(route);
        } else {
            DEBUG_PRINT("Failed to add route: invalid data");
        }
    }
    
    // Получить маршрут по имени
    const Route* GetRoute(const std::string& name) const {
        DEBUG_PRINT("GetRoute: " << name);
        return Get(name);
    }
    
    // Создать и добавить маршрут
    void AddRoute(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip = false) {
        DEBUG_PRINT("AddRoute: " << name << " with " << stop_names.size() << " stop names, roundtrip: " << is_roundtrip);
        auto route = std::make_unique<Route>();
        route->name = name;
        route->is_roundtrip = is_roundtrip;
        
        // Добавляем остановки в маршрут
        for (const auto& stop_name : stop_names) {
            if (stop_container_ && stop_container_->Exists(stop_name)) {
                route->stops.push_back(stop_container_->GetStop(stop_name));
                DEBUG_PRINT("Added stop '" << stop_name << "' to route '" << name << "'");
            } else {
                DEBUG_PRINT("Warning: stop '" << stop_name << "' not found for route '" << name << "'");
            }
        }
        
        // Для некольцевых маршрутов добавляем обратный путь
        if (!is_roundtrip && stop_names.size() > 1) {
            DEBUG_PRINT("Adding return journey for non-circular route");
            
            // Добавляем остановки в обратном порядке, начиная с предпоследней
            for (int i = static_cast<int>(stop_names.size()) - 2; i >= 0; --i) {
                const auto& stop_name = stop_names[i];
                if (stop_container_ && stop_container_->Exists(stop_name)) {
                    route->stops.push_back(stop_container_->GetStop(stop_name));
                    DEBUG_PRINT("Added return stop '" << stop_name << "' to route '" << name << "'");
                }
            }
        }
        
        Add(std::move(route));
    }
    
    // Получить все маршруты
    std::vector<const Route*> GetAllRoutes() const {
        DEBUG_PRINT("GetAllRoutes");
        std::vector<const Route*> result;
        result.reserve(items_.size());
        for (const auto& [name, item] : items_) {
            result.push_back(item.get());
        }
        return result;
    }

private:
    StopContainer* stop_container_; // Ссылка на контейнер остановок
};

} // namespace domain
