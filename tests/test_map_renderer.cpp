#include "test_map_renderer.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <variant>

namespace test_map_renderer {

void TestRenderSettings() {
    std::cout << "  TestRenderSettings... ";
    
    map_renderer::RenderSettings settings;
    
    // Тест установки базовых параметров
    settings.width = 1200;
    settings.height = 800;
    settings.padding = 50;
    settings.line_width = 14;
    settings.stop_radius = 5;
    settings.bus_label_font_size = 20;
    settings.bus_label_offset = map_renderer::Offset{7, 15};
    settings.stop_label_font_size = 18;
    settings.stop_label_offset = map_renderer::Offset{7, -3};
    settings.underlayer_color = map_renderer::Color("white");
    settings.underlayer_width = 3;
    settings.color_palette = {map_renderer::Color("green"), map_renderer::Color("red"), map_renderer::Color("blue")};
    
    // Проверяем, что параметры установлены корректно
    assert(settings.width == 1200);
    assert(settings.height == 800);
    assert(settings.padding == 50);
    assert(settings.line_width == 14);
    assert(settings.stop_radius == 5);
    assert(settings.bus_label_font_size == 20);
    assert(settings.bus_label_offset.dx == 7);
    assert(settings.bus_label_offset.dy == 15);
    assert(settings.stop_label_font_size == 18);
    assert(settings.stop_label_offset.dx == 7);
    assert(settings.stop_label_offset.dy == -3);
    assert(settings.underlayer_width == 3);
    assert(settings.color_palette.size() == 3);
    
    std::cout << "PASSED" << std::endl;
}

void TestSphereProjector() {
    std::cout << "  TestSphereProjector... ";
    
    std::vector<geo::Coordinates> coords = {
        {55.611087, 37.20829},
        {55.595884, 37.209755},
        {55.632761, 37.333324}
    };
    
    map_renderer::SphereProjector projector(coords.begin(), coords.end(), 1200, 800, 50);
    
    // Тестируем проецирование координат
    svg::Point point = projector({55.611087, 37.20829});
    
    // Проверяем, что координаты проецируются корректно
    assert(point.x >= 0);
    assert(point.y >= 0);
    
    std::cout << "PASSED" << std::endl;
}

void TestSvgElements() {
    std::cout << "  TestSvgElements... ";
    
    // Тест создания SVG элементов
    svg::Document doc;
    
    // Добавляем круг
    svg::Circle circle;
    circle.SetCenter({100, 100}).SetRadius(50).SetFillColor("red");
    doc.Add(circle);
    
    // Добавляем текст
    svg::Text text;
    text.SetPosition({100, 100}).SetData("Test").SetFillColor("black");
    doc.Add(text);
    
    // Рендерим в строку
    std::ostringstream oss;
    doc.Render(oss);
    std::string svg_output = oss.str();
    
    // Проверяем наличие основных элементов
    assert(svg_output.find("circle") != std::string::npos);
    assert(svg_output.find("text") != std::string::npos);
    assert(svg_output.find("Test") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void TestMapRendering() {
    std::cout << "  TestMapRendering... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"A", {55.611087, 37.20829}},
        {"B", {55.595884, 37.209755}},
        {"C", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем маршрут
    std::vector<std::string> route_stops = {"A", "B", "C"};
    catalogue.AddRoute("Bus1", route_stops, false);
    
    map_renderer::RenderSettings settings;
    settings.width = 1200;
    settings.height = 800;
    settings.padding = 50;
    settings.line_width = 14;
    settings.stop_radius = 5;
    settings.bus_label_font_size = 20;
    settings.bus_label_offset = map_renderer::Offset{7, 15};
    settings.stop_label_font_size = 18;
    settings.stop_label_offset = map_renderer::Offset{7, -3};
    settings.underlayer_color = map_renderer::Color("white");
    settings.underlayer_width = 3;
    settings.color_palette = {map_renderer::Color("green"), map_renderer::Color("red"), map_renderer::Color("blue")};
    
    // Создаем рендерер
    map_renderer::Render renderer(settings);
    
    // Рендерим карту
    std::ostringstream oss;
    renderer.RenderMap(catalogue, oss);
    std::string svg_output = oss.str();
    
    // Проверяем, что SVG содержит основные элементы
    assert(svg_output.find("<?xml") != std::string::npos);
    assert(svg_output.find("<svg") != std::string::npos);
    assert(svg_output.find("</svg>") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void TestColorPalette() {
    std::cout << "  TestColorPalette... ";
    
    map_renderer::RenderSettings settings;
    settings.color_palette = {map_renderer::Color("green"), map_renderer::Color("red"), map_renderer::Color("blue"), map_renderer::Color("yellow")};
    
    // Тест получения цвета по индексу
    assert(std::get<std::string>(settings.color_palette[0].value) == "green");
    assert(std::get<std::string>(settings.color_palette[1].value) == "red");
    assert(std::get<std::string>(settings.color_palette[2].value) == "blue");
    assert(std::get<std::string>(settings.color_palette[3].value) == "yellow");
    
    // Тест циклического доступа
    assert(std::get<std::string>(settings.color_palette[4 % settings.color_palette.size()].value) == "green");
    assert(std::get<std::string>(settings.color_palette[5 % settings.color_palette.size()].value) == "red");
    
    std::cout << "PASSED" << std::endl;
}

void TestRouteRendering() {
    std::cout << "  TestRouteRendering... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем остановки
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"A", {55.611087, 37.20829}},
        {"B", {55.595884, 37.209755}},
        {"C", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    std::vector<std::string> route_stops = {"A", "B", "C"};
    catalogue.AddRoute("Bus1", route_stops, false);
    
    map_renderer::RenderSettings settings;
    settings.width = 1200;
    settings.height = 800;
    settings.padding = 50;
    settings.line_width = 14;
    settings.color_palette = {map_renderer::Color("green")};
    
    map_renderer::Render renderer(settings);
    
    std::ostringstream oss;
    renderer.RenderMap(catalogue, oss);
    std::string svg_output = oss.str();
    
    // Проверяем наличие polyline для маршрута
    assert(svg_output.find("polyline") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void TestStopRendering() {
    std::cout << "  TestStopRendering... ";
    
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
    settings.stop_radius = 5;
    settings.color_palette = {map_renderer::Color("green")};
    
    map_renderer::Render renderer(settings);
    
    std::ostringstream oss;
    renderer.RenderMap(catalogue, oss);
    std::string svg_output = oss.str();
    
    // Проверяем наличие circle для остановок
    assert(svg_output.find("circle") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void TestLabelRendering() {
    std::cout << "  TestLabelRendering... ";
    
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
    settings.bus_label_font_size = 20;
    settings.bus_label_offset = map_renderer::Offset{7, 15};
    settings.stop_label_font_size = 18;
    settings.stop_label_offset = map_renderer::Offset{7, -3};
    settings.color_palette = {map_renderer::Color("green")};
    
    map_renderer::Render renderer(settings);
    
    std::ostringstream oss;
    renderer.RenderMap(catalogue, oss);
    std::string svg_output = oss.str();
    
    // Проверяем наличие text для меток
    assert(svg_output.find("text") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void TestComplexMap() {
    std::cout << "  TestComplexMap... ";
    
    transport_catalogue::TransportCatalogue catalogue;
    
    // Добавляем множество остановок
    std::vector<std::pair<std::string, std::pair<double, double>>> stops = {
        {"A", {55.611087, 37.20829}},
        {"B", {55.595884, 37.209755}},
        {"C", {55.632761, 37.333324}},
        {"D", {55.632761, 37.333324}},
        {"E", {55.632761, 37.333324}}
    };
    catalogue.AddStops(stops);
    
    // Добавляем несколько маршрутов
    std::vector<std::string> route1 = {"A", "B", "C"};
    std::vector<std::string> route2 = {"B", "C", "D"};
    std::vector<std::string> route3 = {"A", "C", "E"};
    
    catalogue.AddRoute("Bus1", route1, false);
    catalogue.AddRoute("Bus2", route2, false);
    catalogue.AddRoute("Bus3", route3, true); // кольцевой
    
    map_renderer::RenderSettings settings;
    settings.width = 1200;
    settings.height = 800;
    settings.padding = 50;
    settings.line_width = 14;
    settings.stop_radius = 5;
    settings.bus_label_font_size = 20;
    settings.bus_label_offset = map_renderer::Offset{7, 15};
    settings.stop_label_font_size = 18;
    settings.stop_label_offset = map_renderer::Offset{7, -3};
    settings.underlayer_color = map_renderer::Color("white");
    settings.underlayer_width = 3;
    settings.color_palette = {map_renderer::Color("green"), map_renderer::Color("red"), map_renderer::Color("blue"), map_renderer::Color("yellow"), map_renderer::Color("purple")};
    
    map_renderer::Render renderer(settings);
    
    std::ostringstream oss;
    renderer.RenderMap(catalogue, oss);
    std::string svg_output = oss.str();
    
    // Проверяем наличие всех элементов
    assert(svg_output.find("polyline") != std::string::npos);
    assert(svg_output.find("circle") != std::string::npos);
    assert(svg_output.find("text") != std::string::npos);
    
    std::cout << "PASSED" << std::endl;
}

void RunAllTests() {
    TestRenderSettings();
    TestSphereProjector();
    TestSvgElements();
    TestMapRendering();
    TestColorPalette();
    TestRouteRendering();
    TestStopRendering();
    TestLabelRendering();
    TestComplexMap();
}

} // namespace test_map_renderer 