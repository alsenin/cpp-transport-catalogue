#pragma once

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#endif
#define DEBUG_PRINT(x) \
    do                 \
    {                  \
    } while (0)

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace request_handler
{

    // Базовый класс для всех запросов
    class Request
    {
    public:
        virtual ~Request() = default;
        virtual json::Node Execute(const transport_catalogue::TransportCatalogue &catalogue) const = 0;
        virtual std::string GetType() const = 0;
    };

    // Конкретные запросы
    class StopRequest : public Request
    {
    public:
        StopRequest(const std::string &name, int id) : name_(name), id_(id) {}

        json::Node Execute(const transport_catalogue::TransportCatalogue &catalogue) const override;
        std::string GetType() const override { return "Stop"; }

    private:
        std::string name_;
        int id_;
    };

    class BusRequest : public Request
    {
    public:
        BusRequest(const std::string &name, int id) : name_(name), id_(id) {}

        json::Node Execute(const transport_catalogue::TransportCatalogue &catalogue) const override;
        std::string GetType() const override { return "Bus"; }

    private:
        std::string name_;
        int id_;
    };

    class MapRequest : public Request
    {
    public:
        MapRequest(int id, const map_renderer::Render &renderer) : id_(id), renderer_(renderer) {}

        json::Node Execute(const transport_catalogue::TransportCatalogue &catalogue) const override;
        std::string GetType() const override { return "Map"; }

    private:
        int id_;
        map_renderer::Render renderer_;
    };

    // Реестр запросов
    class RequestRegistry
    {
    public:
        using RequestCreator = std::function<std::unique_ptr<Request>(const json::Dict &, const map_renderer::Render &)>;

        void Register(const std::string &type, RequestCreator creator);
        std::unique_ptr<Request> Create(const std::string &type, const json::Dict &request_dict, const map_renderer::Render &renderer) const;

    private:
        std::unordered_map<std::string, RequestCreator> creators_;
    };

    // Фабрика запросов
    class RequestFactory
    {
    public:
        static std::unique_ptr<Request> CreateStopRequest(const json::Dict &request_dict, const map_renderer::Render &renderer);
        static std::unique_ptr<Request> CreateBusRequest(const json::Dict &request_dict, const map_renderer::Render &renderer);
        static std::unique_ptr<Request> CreateMapRequest(const json::Dict &request_dict, const map_renderer::Render &renderer);
    };

    class RequestHandler
    {
    public:
        explicit RequestHandler(transport_catalogue::TransportCatalogue &catalogue);

        // Обработка JSON документа (основной метод)
        void ProcessDocument(const json::Document &document);

        // Обработка запросов из JSON документа
        json::Document ProcessRequests(const json::Document &document);

    private:
        // Регистрация типов запросов
        void RegisterRequestTypes();

        // Обработка одного запроса
        json::Node ProcessSingleRequest(const json::Dict &request_dict);

        // Обработка статистических запросов
        void ProcessStatRequests(const json::Node &stat_requests);

    private:
        transport_catalogue::TransportCatalogue &catalogue_;
        json_reader::JsonReader json_reader_;
        map_renderer::Render renderer_;
        RequestRegistry request_registry_;
    };

} // namespace request_handler
