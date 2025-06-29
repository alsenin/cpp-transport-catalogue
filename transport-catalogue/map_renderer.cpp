#include "map_renderer.h"
#include <unordered_set>
#include <sstream>


namespace map_renderer {

void Render::RenderMap(const transport_catalogue::TransportCatalogue& catalogue, std::ostream& out) const {
    // Создаем SVG документ
    svg::Document doc;
    
    // Собираем координаты остановок, которые фактически используются в маршрутах
    std::vector<geo::Coordinates> all_coordinates;
    std::unordered_set<std::string> used_stops;
    
    // Получаем все маршруты и собираем используемые остановки
    auto routes = catalogue.GetRouteContainer().GetAllRoutes();
    for (const auto& route : routes) {
        for (const auto& stop : route->stops) {
            if (stop && (stop->coordinates.lat != 0.0 || stop->coordinates.lng != 0.0)) {
                used_stops.insert(stop->name);
            }
        }
    }
    
    // Добавляем координаты только используемых остановок
    auto all_stops = catalogue.GetStopContainer().GetAllStops();
    for (const auto& stop : all_stops) {
        if (used_stops.count(stop->name) > 0) {
            all_coordinates.push_back(stop->coordinates);
        }
    }
    
    // Создаем проектор
    SphereProjector projector(all_coordinates.begin(), all_coordinates.end(),
                             settings_.width, settings_.height, settings_.padding);
    
    // Если нет валидных координат, выводим пустой SVG
    if (all_coordinates.empty()) {
        doc.Render(out);
        return;
    }
    
    // Рендерим только линии маршрутов
    RenderBusLines(catalogue, projector, doc);
    
    // Рендерим названия маршрутов
    RenderBusLabels(catalogue, projector, doc);
    
    // Рендерим символы остановок
    RenderStopSymbols(catalogue, projector, doc);
    
    // Рендерим названия остановок
    RenderStopLabels(catalogue, projector, doc);
    
    // Выводим SVG документ
    doc.Render(out);
}

std::string Render::ColorToString(const map_renderer::Color& color) const {
    if (auto str = std::get_if<std::string>(&color.value)) {
        return *str;
    } else if (auto rgb = std::get_if<std::vector<int>>(&color.value)) {
        return "rgb(" + std::to_string((*rgb)[0]) + "," +
               std::to_string((*rgb)[1]) + "," + std::to_string((*rgb)[2]) + ")";
    } else if (auto rgba = std::get_if<std::vector<double>>(&color.value)) {
        // Форматируем opacity с точностью до 6 знаков после запятой
        std::ostringstream oss;
//        oss.precision(6);
//        oss << std::fixed << (*rgba)[3];
        oss << std::defaultfloat << (*rgba)[3];
        std::string opacity_str = oss.str();
        // Убираем незначащие нули и точку
        while (!opacity_str.empty() && opacity_str.back() == '0') {
            opacity_str.pop_back();
        }
        if (!opacity_str.empty() && opacity_str.back() == '.') {
            opacity_str.pop_back();
        }
        return "rgba(" + std::to_string(static_cast<int>((*rgba)[0])) + "," +
               std::to_string(static_cast<int>((*rgba)[1])) + "," +
               std::to_string(static_cast<int>((*rgba)[2])) + "," +
               opacity_str + ")";
    }
    return "black"; // fallback
}

void Render::RenderBusLines(const transport_catalogue::TransportCatalogue& catalogue,
                           const SphereProjector& projector, svg::Document& doc) const {
    auto routes = catalogue.GetRouteContainer().GetAllRoutes();
    
    // Сортируем маршруты по названиям в лексикографическом порядке
    std::sort(routes.begin(), routes.end(), 
              [](const Route* a, const Route* b) { 
                  return a->name.compare(b->name) < 0;
              });
    
    size_t color_index = 0;
    
    for (const auto& route : routes) {
        // Пропускаем маршруты без остановок
        if (route->stops.empty()) continue;
        
        // Создаем полилинию для маршрута
        svg::Polyline polyline;
        
        // Добавляем точки маршрута в порядке следования
        for (const auto& stop : route->stops) {
            if (stop && (stop->coordinates.lat != 0.0 || stop->coordinates.lng != 0.0)) {
                svg::Point point = projector(stop->coordinates);
                
                // Добавляем все точки без фильтрации дубликатов
                polyline.AddPoint(point);
            }
        }
        
        // Настраиваем стиль линии согласно требованиям
        std::string stroke_color;
        if (settings_.color_palette.empty()) {
            stroke_color = "black"; // fallback цвет при пустой палитре
        } else {
            stroke_color = ColorToString(settings_.color_palette[color_index % settings_.color_palette.size()]);
        }
        polyline.SetStrokeColor(stroke_color);
        polyline.SetFillColor("none");
        polyline.SetStrokeWidth(settings_.line_width);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        doc.Add(polyline);
        color_index++;
    }
}

void Render::RenderBusLabels(const transport_catalogue::TransportCatalogue& catalogue,
                            const SphereProjector& projector, svg::Document& doc) const {
    auto routes = catalogue.GetRouteContainer().GetAllRoutes();
    
    // Сортируем маршруты по названиям в алфавитном порядке
    std::sort(routes.begin(), routes.end(), 
              [](const Route* a, const Route* b) { 
                  return a->name.compare(b->name) < 0;
              });
    
    size_t color_index = 0;
    
    for (const auto& route : routes) {
        // Пропускаем маршруты без остановок
        if (route->stops.empty()) continue;
        
        // Получаем цвет маршрута
        std::string route_color;
        if (settings_.color_palette.empty()) {
            route_color = "black"; // fallback цвет при пустой палитре
        } else {
            route_color = ColorToString(settings_.color_palette[color_index % settings_.color_palette.size()]);
        }
        
        // Определяем конечные остановки
        std::vector<const Stop*> terminal_stops;
        
        if (route->is_roundtrip) {
            // Для кольцевого маршрута конечная - первая остановка
            if (!route->stops.empty()) {
                terminal_stops.push_back(route->stops.front());
            }
        } else {
            // Для некольцевого маршрута конечные - первая и последняя из оригинального маршрута
            if (!route->stops.empty()) {
                size_t original_size = (route->stops.size() + 1) / 2; // Округляем вверх
                terminal_stops.push_back(route->stops.front());
                if (original_size > 1 && route->stops.front() != route->stops[original_size - 1]) {
                    terminal_stops.push_back(route->stops[original_size - 1]);
                }
            }
        }
        
        // Рендерим названия для каждой конечной остановки
        for (const auto& stop : terminal_stops) {
            if (stop && (stop->coordinates.lat != 0.0 || stop->coordinates.lng != 0.0)) {
                svg::Point point = projector(stop->coordinates);
                
                // Создаем подложку (background)
                svg::Text background;
                background.SetPosition(point);
                background.SetOffset({settings_.bus_label_offset.dx, settings_.bus_label_offset.dy});
                background.SetFontSize(settings_.bus_label_font_size);
                background.SetFontFamily("Verdana");
                background.SetFontWeight("bold");
                background.SetData(route->name);
                background.SetFillColor(ColorToString(settings_.underlayer_color));
                background.SetStrokeColor(ColorToString(settings_.underlayer_color));
                background.SetStrokeWidth(settings_.underlayer_width);
                background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
                // Создаем основную надпись
                svg::Text label;
                label.SetPosition(point);
                label.SetOffset({settings_.bus_label_offset.dx, settings_.bus_label_offset.dy});
                label.SetFontSize(settings_.bus_label_font_size);
                label.SetFontFamily("Verdana");
                label.SetFontWeight("bold");
                label.SetData(route->name);
                label.SetFillColor(route_color);
                
                // Добавляем в документ (сначала подложку, потом надпись)
                doc.Add(background);
                doc.Add(label);
            }
        }
        
        color_index++;
    }
}

void Render::RenderStopSymbols(const transport_catalogue::TransportCatalogue& catalogue,
                              const SphereProjector& projector, svg::Document& doc) const {
    // Собираем все остановки, которые используются в маршрутах
    std::unordered_set<std::string> used_stops;
    auto routes = catalogue.GetRouteContainer().GetAllRoutes();
    
    for (const auto& route : routes) {
        for (const auto& stop : route->stops) {
            if (stop && (stop->coordinates.lat != 0.0 || stop->coordinates.lng != 0.0)) {
                used_stops.insert(stop->name);
            }
        }
    }
    
    // Получаем все остановки и фильтруем только используемые
    std::vector<const Stop*> stops_to_render;
    auto all_stops = catalogue.GetStopContainer().GetAllStops();
    
    for (const auto& stop : all_stops) {
        if (used_stops.count(stop->name) > 0) {
            stops_to_render.push_back(stop);
        }
    }
    
    // Сортируем остановки по названию в лексикографическом порядке
    std::sort(stops_to_render.begin(), stops_to_render.end(),
              [](const Stop* a, const Stop* b) {
                  return a->name.compare(b->name) < 0;
              });
    
    // Рендерим символы остановок
    for (const auto& stop : stops_to_render) {
        svg::Point point = projector(stop->coordinates);
        
        svg::Circle circle;
        circle.SetCenter(point);
        circle.SetRadius(settings_.stop_radius);
        circle.SetFillColor("white");
        
        doc.Add(circle);
    }
}

void Render::RenderStopLabels(const transport_catalogue::TransportCatalogue& catalogue,
                             const SphereProjector& projector, svg::Document& doc) const {
    // Собираем все остановки, которые используются в маршрутах
    std::unordered_set<std::string> used_stops;
    auto routes = catalogue.GetRouteContainer().GetAllRoutes();
    
    for (const auto& route : routes) {
        for (const auto& stop : route->stops) {
            if (stop && (stop->coordinates.lat != 0.0 || stop->coordinates.lng != 0.0)) {
                used_stops.insert(stop->name);
            }
        }
    }
    
    // Получаем все остановки и фильтруем только используемые
    std::vector<const Stop*> stops_to_render;
    auto all_stops = catalogue.GetStopContainer().GetAllStops();
    
    for (const auto& stop : all_stops) {
        if (used_stops.count(stop->name) > 0) {
            stops_to_render.push_back(stop);
        }
    }
    
    // Сортируем остановки по названию в лексикографическом порядке
    std::sort(stops_to_render.begin(), stops_to_render.end(),
              [](const Stop* a, const Stop* b) {
                  return a->name.compare(b->name) < 0;
              });
    
    // Рендерим названия остановок
    for (const auto& stop : stops_to_render) {
        svg::Point point = projector(stop->coordinates);
        
        // Создаем подложку (background)
        svg::Text background;
        background.SetPosition(point);
        background.SetOffset({settings_.stop_label_offset.dx, settings_.stop_label_offset.dy});
        background.SetFontSize(settings_.stop_label_font_size);
        background.SetFontFamily("Verdana");
        background.SetData(stop->name);
        background.SetFillColor(ColorToString(settings_.underlayer_color));
        background.SetStrokeColor(ColorToString(settings_.underlayer_color));
        background.SetStrokeWidth(settings_.underlayer_width);
        background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        // Создаем основную надпись
        svg::Text label;
        label.SetPosition(point);
        label.SetOffset({settings_.stop_label_offset.dx, settings_.stop_label_offset.dy});
        label.SetFontSize(settings_.stop_label_font_size);
        label.SetFontFamily("Verdana");
        label.SetData(stop->name);
        label.SetFillColor("black");
        
        // Добавляем в документ (сначала подложку, потом надпись)
        doc.Add(background);
        doc.Add(label);
    }
}

} // namespace map_renderer