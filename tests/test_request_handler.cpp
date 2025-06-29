#include "test_request_handler.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace test_request_handler {

void TestRequestCreation() {
    std::cout << "  TestRequestCreation... ";
    
    map_renderer::RenderSettings settings;
    settings.color_palette = {map_renderer::Color("green")};
    map_renderer::Render renderer(settings);
    
    // Тест создания запроса остановки
    json::Dict stop_dict = {{"name", json::Node("TestStop")}, {"id", json::Node(1)}};
    auto stop_request = request_handler::RequestFactory::CreateStopRequest(stop_dict, renderer);
    assert(stop_request != nullptr);
    assert(stop_request->GetType() == "Stop");
    
    // Тест создания запроса маршрута
    json::Dict bus_dict = {{"name", json::Node("TestBus")}, {"id", json::Node(2)}};
    auto bus_request = request_handler::RequestFactory::CreateBusRequest(bus_dict, renderer);
    assert(bus_request != nullptr);
    assert(bus_request->GetType() == "Bus");
    
    // Тест создания запроса карты
    json::Dict map_dict = {{"id", json::Node(3)}};
    auto map_request = request_handler::RequestFactory::CreateMapRequest(map_dict, renderer);
    assert(map_request != nullptr);
    assert(map_request->GetType() == "Map");
    
    std::cout << "PASSED" << std::endl;
}

void TestStopRequest() {
    std::cout << "  TestStopRequest... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}}
    };
    catalogue.AddStops(stops);
    
    std::vector<std::string> route_stops = {"Stop1", "Stop2"};
    catalogue.AddRoute("Bus1", route_stops, false);
    
    request_handler::StopRequest stop_req("Stop1", 1);
    json::Node response = stop_req.Execute(catalogue);
    
    // Проверяем, что ответ содержит правильную структуру
    assert(response.IsDict());
    json::Dict response_dict = response.AsMap();
    assert(response_dict.find("request_id") != response_dict.end());
    assert(response_dict["request_id"].AsInt() == 1);
    
    // Проверяем успешный ответ
    assert(response_dict.find("buses") != response_dict.end());
    json::Array buses = response_dict["buses"].AsArray();
    assert(buses.size() == 1);
    assert(buses[0].AsString() == "Bus1");
    
    // Тест несуществующей остановки
    request_handler::StopRequest non_existent_req("NonExistent", 2);
    json::Node error_response = non_existent_req.Execute(catalogue);
    json::Dict error_dict = error_response.AsMap();
    assert(error_dict.find("error_message") != error_dict.end());
    assert(error_dict["error_message"].AsString() == "not found");
    
    std::cout << "PASSED" << std::endl;
}

void TestBusRequest() {
    std::cout << "  TestBusRequest... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем расстояния
    std::vector<std::tuple<std::string, std::string, double>> distances = {
        {"Stop1", "Stop2", 1000},
        {"Stop2", "Stop3", 1500},
        {"Stop3", "Stop2", 1500},
        {"Stop2", "Stop1", 1000}
    };
    catalogue.AddDistances(distances);
    
    std::vector<std::string> route_stops = {"Stop1", "Stop2", "Stop3"};
    catalogue.AddRoute("Bus1", route_stops, false);
    
    // Проверяем BusRequest для некольцевого маршрута
    json::Dict bus_request_dict = {{"type", json::Node("Bus")}, {"name", json::Node("Bus1")}, {"id", json::Node(2)}};
    map_renderer::RenderSettings settings;
    settings.width = 1200;
    settings.height = 800;
    settings.padding = 50;
    settings.color_palette = {map_renderer::Color("green")};
    map_renderer::Render renderer(settings);
    auto bus_request = request_handler::RequestFactory::CreateBusRequest(bus_request_dict, renderer);
    assert(bus_request != nullptr);
    assert(bus_request->GetType() == "Bus");
    
    json::Node response = bus_request->Execute(catalogue);
    assert(response.IsDict());
    const auto& response_dict = response.AsMap();
    assert(response_dict.at("request_id").AsInt() == 2);
    assert(response_dict.at("route_length").AsInt() == 5000);
    assert(response_dict.at("stop_count").AsInt() == 5);
    assert(response_dict.at("unique_stop_count").AsInt() == 3);
    
    // Тест несуществующего маршрута
    request_handler::BusRequest non_existent_req("NonExistent", 2);
    json::Node error_response = non_existent_req.Execute(catalogue);
    json::Dict error_dict = error_response.AsMap();
    assert(error_dict.find("error_message") != error_dict.end());
    assert(error_dict["error_message"].AsString() == "not found");
    
    std::cout << "PASSED" << std::endl;
}

void TestMapRequest() {
    std::cout << "  TestMapRequest... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}}
    };
    catalogue.AddStops(stops);
    
    std::vector<std::string> route_stops = {"Stop1", "Stop2"};
    catalogue.AddRoute("Bus1", route_stops, false);
    
    map_renderer::RenderSettings settings;
    settings.width = 1200;
    settings.height = 800;
    settings.padding = 50;
    settings.color_palette = {map_renderer::Color("green")};
    
    map_renderer::Render renderer(settings);
    request_handler::MapRequest map_req(1, renderer);
    json::Node response = map_req.Execute(catalogue);
    
    // Проверяем структуру ответа
    assert(response.IsDict());
    json::Dict response_dict = response.AsMap();
    assert(response_dict.find("request_id") != response_dict.end());
    assert(response_dict["request_id"].AsInt() == 1);
    
    // Проверяем наличие SVG карты
    assert(response_dict.find("map") != response_dict.end());
    std::string svg_content = response_dict["map"].AsString();
    assert(svg_content.find("<?xml") != std::string::npos);
    assert(svg_content.find("<svg") != std::string::npos);
    assert(svg_content.find("</svg>") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void TestRequestRegistry() {
    std::cout << "  TestRequestRegistry... ";
    
    request_handler::RequestRegistry registry;
    
    // Регистрируем создатели запросов
    registry.Register("Stop", request_handler::RequestFactory::CreateStopRequest);
    registry.Register("Bus", request_handler::RequestFactory::CreateBusRequest);
    registry.Register("Map", request_handler::RequestFactory::CreateMapRequest);
    
    // Создаем запросы
    json::Dict stop_dict = {{"type", json::Node("Stop")}, {"name", json::Node("TestStop")}, {"id", json::Node(1)}};
    map_renderer::RenderSettings settings;
    map_renderer::Render renderer(settings);
    
    auto stop_request = registry.Create("Stop", stop_dict, renderer);
    assert(stop_request != nullptr);
    assert(stop_request->GetType() == "Stop");
    
    json::Dict bus_dict = {{"type", json::Node("Bus")}, {"name", json::Node("TestBus")}, {"id", json::Node(2)}};
    auto bus_request = registry.Create("Bus", bus_dict, renderer);
    assert(bus_request != nullptr);
    assert(bus_request->GetType() == "Bus");
    
    json::Dict map_dict = {{"type", json::Node("Map")}, {"id", json::Node(3)}};
    auto map_request = registry.Create("Map", map_dict, renderer);
    assert(map_request != nullptr);
    assert(map_request->GetType() == "Map");
    
    // Тест неизвестного типа
    try {
        registry.Create("Unknown", stop_dict, renderer);
        assert(false); // Не должно дойти до сюда
    } catch (const json::ParsingError&) {
        // Ожидаемое исключение
    }
    
    std::cout << "PASSED" << std::endl;
}

void TestRequestFactory() {
    std::cout << "  TestRequestFactory... ";
    
    map_renderer::RenderSettings settings;
    map_renderer::Render renderer(settings);
    
    // Тест создания запроса остановки
    json::Dict stop_dict = {{"name", json::Node("TestStop")}, {"id", json::Node(1)}};
    auto stop_request = request_handler::RequestFactory::CreateStopRequest(stop_dict, renderer);
    assert(stop_request != nullptr);
    assert(stop_request->GetType() == "Stop");
    
    // Тест создания запроса маршрута
    json::Dict bus_dict = {{"name", json::Node("TestBus")}, {"id", json::Node(2)}};
    auto bus_request = request_handler::RequestFactory::CreateBusRequest(bus_dict, renderer);
    assert(bus_request != nullptr);
    assert(bus_request->GetType() == "Bus");
    
    // Тест создания запроса карты
    json::Dict map_dict = {{"id", json::Node(3)}};
    auto map_request = request_handler::RequestFactory::CreateMapRequest(map_dict, renderer);
    assert(map_request != nullptr);
    assert(map_request->GetType() == "Map");
    
    std::cout << "PASSED" << std::endl;
}

void TestRequestHandler() {
    std::cout << "  TestRequestHandler... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    request_handler::RequestHandler handler(catalogue);
    
    // Проверяем, что обработчик создался корректно
    // (основная функциональность тестируется в других тестах)
    
    std::cout << "PASSED" << std::endl;
}

void TestJsonProcessing() {
    std::cout << "  TestJsonProcessing... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    request_handler::RequestHandler handler(catalogue);
    
    // Создаем базовый запрос
    json::Dict base_request = {
        {"base_requests", json::Node(json::Array{})},
        {"stat_requests", json::Node(json::Array{
            json::Node(json::Dict{{"type", json::Node("Stop")}, {"name", json::Node("TestStop")}, {"id", json::Node(1)}})
        })}
    };
    
    json::Document document = json::Document(json::Node(base_request));
    
    // Обрабатываем документ
    handler.ProcessDocument(document);
    json::Document response = handler.ProcessRequests(document);
    
    // Проверяем, что вызов обработчика не приводит к исключениям и завершается корректно
    // (Результат выводится в std::cout, возвращаемое значение — пустой объект)
    (void)response; // подавляем предупреждение о неиспользуемой переменной
    
    std::cout << "PASSED" << std::endl;
}

void TestErrorHandling() {
    std::cout << "  TestErrorHandling... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    request_handler::RequestHandler handler(catalogue);
    
    // Тест обработки некорректного JSON
    try {
        std::istringstream invalid_stream("invalid json");
        json::Document invalid_doc = json::Load(invalid_stream);
        assert(false); // Не должно дойти до сюда
    } catch (const json::ParsingError&) {
        // Ожидаемое исключение
    }
    
    // Тест обработки некорректного запроса
    json::Dict invalid_request = {
        {"type", json::Node("InvalidType")},
        {"id", json::Node(1)}
    };
    
    json::Document doc = json::Document(json::Node(invalid_request));
    
    // ProcessDocument может просто игнорировать некорректные запросы
    // или обрабатывать их без исключений
    handler.ProcessDocument(doc);
    
    std::cout << "PASSED" << std::endl;
}

void TestComplexRequests() {
    std::cout << "  TestComplexRequests... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    request_handler::RequestHandler handler(catalogue);
    
    // Создаем сложный запрос с несколькими типами
    json::Dict base_request = {
        {"base_requests", json::Node(json::Array{
            json::Node(json::Dict{{"type", json::Node("Stop")}, {"name", json::Node("Stop1")}, {"latitude", json::Node(55.611087)}, {"longitude", json::Node(37.20829)}}),
            json::Node(json::Dict{{"type", json::Node("Stop")}, {"name", json::Node("Stop2")}, {"latitude", json::Node(55.595884)}, {"longitude", json::Node(37.209755)}}),
            json::Node(json::Dict{{"type", json::Node("Bus")}, {"name", json::Node("Bus1")}, {"stops", json::Node(json::Array{json::Node("Stop1"), json::Node("Stop2")})}, {"is_roundtrip", json::Node(false)}})
        })},
        {"stat_requests", json::Node(json::Array{
            json::Node(json::Dict{{"type", json::Node("Stop")}, {"name", json::Node("Stop1")}, {"id", json::Node(1)}}),
            json::Node(json::Dict{{"type", json::Node("Bus")}, {"name", json::Node("Bus1")}, {"id", json::Node(2)}}),
            json::Node(json::Dict{{"type", json::Node("Map")}, {"id", json::Node(3)}})
        })}
    };
    
    json::Document document = json::Document(json::Node(base_request));
    
    // Обрабатываем документ
    handler.ProcessDocument(document);
    json::Document response = handler.ProcessRequests(document);
    
    // Проверяем, что вызов обработчика не приводит к исключениям и завершается корректно
    // (Результат выводится в std::cout, возвращаемое значение — пустой объект)
    (void)response; // подавляем предупреждение о неиспользуемой переменной
    
    std::cout << "PASSED" << std::endl;
}

void RunAllTests() {
    TestRequestCreation();
    TestStopRequest();
    TestBusRequest();
    TestMapRequest();
    TestRequestRegistry();
    TestRequestFactory();
    TestRequestHandler();
    TestJsonProcessing();
    TestErrorHandling();
    TestComplexRequests();
}

} // namespace test_request_handler 