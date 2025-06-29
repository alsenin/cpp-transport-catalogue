#include "test_transport_catalogue.h"
#include <iostream>
#include <cassert>
#include <cmath>

namespace test_transport_catalogue {

void TestStopOperations() {
    std::cout << "  TestStopOperations... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Тест добавления остановок
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}}
    };
    catalogue.AddStops(stops);
    
    // Тест получения остановки по имени
    const Stop* stop1 = catalogue.GetStopByName("Stop1");
    assert(stop1 != nullptr);
    assert(stop1->name == "Stop1");
    assert(std::abs(stop1->coordinates.lat - 55.611087) < 1e-6);
    assert(std::abs(stop1->coordinates.lng - 37.20829) < 1e-6);
    
    const Stop* stop2 = catalogue.GetStopByName("Stop2");
    assert(stop2 != nullptr);
    assert(stop2->name == "Stop2");
    
    // Тест несуществующей остановки
    const Stop* non_existent = catalogue.GetStopByName("NonExistent");
    assert(non_existent == nullptr);
    
    std::cout << "PASSED" << std::endl;
}

void TestBusOperations() {
    std::cout << "  TestBusOperations... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Тест добавления маршрута
    std::vector<std::string> route1_stops = {"Stop1", "Stop2", "Stop3"};
    catalogue.AddRoute("Bus1", route1_stops);
    
    // Проверяем, что маршрут существует
    assert(catalogue.RouteExists("Bus1"));
    assert(!catalogue.RouteExists("NonExistentBus"));
    
    std::cout << "PASSED" << std::endl;
}

void TestRouteInfo() {
    std::cout << "  TestRouteInfo... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем расстояния между остановками
    std::vector<std::tuple<std::string, std::string, double>> distances = {
        {"Stop1", "Stop2", 1000},
        {"Stop2", "Stop3", 1500}
    };
    catalogue.AddDistances(distances);
    
    // Тест маршрута
    std::vector<std::string> route_stops = {"Stop1", "Stop2", "Stop3"};
    catalogue.AddRoute("Bus1", route_stops);
    
    transport_catalogue::RouteInfo info = catalogue.GetRouteInfo("Bus1");
    
    // Проверяем количество остановок
    assert(info.stops_count == 5); // 1-2-3-2-1 для некольцевого маршрута
    assert(info.unique_stops_count == 3);
    
    // Проверяем длину маршрута (должна быть больше 0)
    assert(info.route_length > 0);
    
    std::cout << "PASSED" << std::endl;
}

void TestDistanceCalculations() {
    std::cout << "  TestDistanceCalculations... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки с известными координатами
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Устанавливаем реальное расстояние
    std::vector<std::tuple<std::string, std::string, double>> distances = {
        {"Stop1", "Stop2", 1000}
    };
    catalogue.AddDistances(distances);
    
    // Добавляем маршрут для проверки длины
    std::vector<std::string> route_stops = {"Stop1", "Stop2"};
    catalogue.AddRoute("Bus1", route_stops);
    
    transport_catalogue::RouteInfo info = catalogue.GetRouteInfo("Bus1");
    
    // Проверяем, что длина маршрута учитывает заданное расстояние
    assert(info.route_length >= 1000);
    
    std::cout << "PASSED" << std::endl;
}

void TestStopInfo() {
    std::cout << "  TestStopInfo... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем маршруты
    std::vector<std::string> route1_stops = {"Stop1", "Stop2"};
    std::vector<std::string> route2_stops = {"Stop2", "Stop3"};
    std::vector<std::string> route3_stops = {"Stop1", "Stop3"};
    
    catalogue.AddRoute("Bus1", route1_stops);
    catalogue.AddRoute("Bus2", route2_stops);
    catalogue.AddRoute("Bus3", route3_stops);
    
    // Проверяем информацию об остановке
    std::vector<std::string> stop1_buses = catalogue.GetStopInfo("Stop1");
    assert(stop1_buses.size() == 2); // Bus1 и Bus3
    assert(std::find(stop1_buses.begin(), stop1_buses.end(), "Bus1") != stop1_buses.end());
    assert(std::find(stop1_buses.begin(), stop1_buses.end(), "Bus3") != stop1_buses.end());
    
    std::vector<std::string> stop2_buses = catalogue.GetStopInfo("Stop2");
    assert(stop2_buses.size() == 2); // Bus1 и Bus2
    assert(std::find(stop2_buses.begin(), stop2_buses.end(), "Bus1") != stop2_buses.end());
    assert(std::find(stop2_buses.begin(), stop2_buses.end(), "Bus2") != stop2_buses.end());
    
    // Проверяем несуществующую остановку
    std::vector<std::string> non_existent_buses = catalogue.GetStopInfo("NonExistent");
    assert(non_existent_buses.empty());
    
    std::cout << "PASSED" << std::endl;
}

void TestRouteExists() {
    std::cout << "  TestRouteExists... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}}
    };
    catalogue.AddStops(stops);
    
    // Проверяем несуществующий маршрут
    assert(!catalogue.RouteExists("NonExistent"));
    
    // Добавляем маршрут
    std::vector<std::string> route_stops = {"Stop1", "Stop2"};
    catalogue.AddRoute("Bus1", route_stops);
    
    // Проверяем существующий маршрут
    assert(catalogue.RouteExists("Bus1"));
    
    std::cout << "PASSED" << std::endl;
}

void TestGetStopByName() {
    std::cout << "  TestGetStopByName... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановку
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"TestStop", {55.611087, 37.20829}}
    };
    catalogue.AddStops(stops);
    
    // Получаем остановку
    const Stop* stop = catalogue.GetStopByName("TestStop");
    assert(stop != nullptr);
    assert(stop->name == "TestStop");
    assert(std::abs(stop->coordinates.lat - 55.611087) < 1e-6);
    assert(std::abs(stop->coordinates.lng - 37.20829) < 1e-6);
    
    // Проверяем несуществующую остановку
    const Stop* non_existent = catalogue.GetStopByName("NonExistent");
    assert(non_existent == nullptr);
    
    std::cout << "PASSED" << std::endl;
}

void TestGetRouteInfo() {
    std::cout << "  TestGetRouteInfo... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Устанавливаем расстояния
    std::vector<std::tuple<std::string, std::string, double>> distances = {
        {"Stop1", "Stop2", 1000},
        {"Stop2", "Stop3", 1500}
    };
    catalogue.AddDistances(distances);
    
    // Добавляем маршрут
    std::vector<std::string> route_stops = {"Stop1", "Stop2", "Stop3"};
    catalogue.AddRoute("Bus1", route_stops);
    
    transport_catalogue::RouteInfo info = catalogue.GetRouteInfo("Bus1");
    
    // Проверяем количество остановок
    assert(info.stops_count == 5); // 1-2-3-2-1
    assert(info.unique_stops_count == 3);
    
    // Проверяем длину маршрута
    assert(info.route_length > 0);
    
    std::cout << "PASSED" << std::endl;
}

void TestGetStopInfo() {
    std::cout << "  TestGetStopInfo... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем маршрут
    std::vector<std::string> route_stops = {"Stop1", "Stop2"};
    catalogue.AddRoute("Bus1", route_stops);
    
    // Проверяем информацию об остановке
    std::vector<std::string> stop1_buses = catalogue.GetStopInfo("Stop1");
    assert(stop1_buses.size() == 1);
    assert(std::find(stop1_buses.begin(), stop1_buses.end(), "Bus1") != stop1_buses.end());
    
    std::vector<std::string> stop2_buses = catalogue.GetStopInfo("Stop2");
    assert(stop2_buses.size() == 1);
    assert(std::find(stop2_buses.begin(), stop2_buses.end(), "Bus1") != stop2_buses.end());
    
    std::cout << "PASSED" << std::endl;
}

void TestComplexScenarios() {
    std::cout << "  TestComplexScenarios... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем несколько остановок
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"Stop1", {55.611087, 37.20829}},
        {"Stop2", {55.595884, 37.209755}},
        {"Stop3", {55.632761, 37.333324}},
        {"Stop4", {55.574371, 37.6517}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем расстояния
    std::vector<std::tuple<std::string, std::string, double>> distances = {
        {"Stop1", "Stop2", 1000},
        {"Stop2", "Stop3", 1500},
        {"Stop3", "Stop4", 2000}
    };
    catalogue.AddDistances(distances);
    
    // Добавляем несколько маршрутов
    std::vector<std::string> route1_stops = {"Stop1", "Stop2", "Stop3"};
    std::vector<std::string> route2_stops = {"Stop2", "Stop3", "Stop4"};
    std::vector<std::string> route3_stops = {"Stop1", "Stop4"};
    
    catalogue.AddRoute("Bus1", route1_stops);
    catalogue.AddRoute("Bus2", route2_stops);
    catalogue.AddRoute("Bus3", route3_stops);
    
    // Проверяем информацию о маршрутах
    transport_catalogue::RouteInfo route1_info = catalogue.GetRouteInfo("Bus1");
    assert(route1_info.stops_count == 5); // 1-2-3-2-1
    assert(route1_info.unique_stops_count == 3);
    
    transport_catalogue::RouteInfo route2_info = catalogue.GetRouteInfo("Bus2");
    assert(route2_info.stops_count == 5); // 2-3-4-3-2
    assert(route2_info.unique_stops_count == 3);
    
    transport_catalogue::RouteInfo route3_info = catalogue.GetRouteInfo("Bus3");
    assert(route3_info.stops_count == 3); // 1-4-1
    assert(route3_info.unique_stops_count == 2);
    
    // Проверяем информацию об остановках
    std::vector<std::string> stop2_buses = catalogue.GetStopInfo("Stop2");
    assert(stop2_buses.size() == 2); // Bus1 и Bus2
    assert(std::find(stop2_buses.begin(), stop2_buses.end(), "Bus1") != stop2_buses.end());
    assert(std::find(stop2_buses.begin(), stop2_buses.end(), "Bus2") != stop2_buses.end());
    
    std::cout << "PASSED" << std::endl;
}

void RunAllTests() {
    TestStopOperations();
    TestBusOperations();
    TestRouteInfo();
    TestDistanceCalculations();
    TestStopInfo();
    TestRouteExists();
    TestGetStopByName();
    TestGetRouteInfo();
    TestGetStopInfo();
    TestComplexScenarios();
}

} // namespace test_transport_catalogue 