#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <optional>
#include <iostream>
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace map_renderer {

// Структура для хранения цвета
struct Color {
    std::variant<std::string, std::vector<int>, std::vector<double>> value;
    
    // Конструкторы для разных типов цветов
    Color(const std::string& name) : value(name) {}
    Color(int r, int g, int b) : value(std::vector<int>{r, g, b}) {}
    Color(int r, int g, int b, double opacity) : value(std::vector<double>{static_cast<double>(r), static_cast<double>(g), static_cast<double>(b), opacity}) {}
};

// Структура для хранения смещения (dx, dy)
struct Offset {
    double dx;
    double dy;
    
    Offset(double dx, double dy) : dx(dx), dy(dy) {}
};

// Основная структура настроек рендеринга
struct RenderSettings {
    double width = 1200.0;
    double height = 1200.0;
    double padding = 50.0;
    double line_width = 14.0;
    double stop_radius = 5.0;
    int bus_label_font_size = 20;
    Offset bus_label_offset{7.0, 15.0};
    int stop_label_font_size = 20;
    Offset stop_label_offset{7.0, -3.0};
    Color underlayer_color{"white"};
    double underlayer_width = 3.0;
    std::vector<Color> color_palette;
};

// Константа для сравнения с нулем
inline const double EPSILON = 1e-6;

// Функция для проверки, является ли значение близким к нулю
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// Класс для проецирования сферических координат на плоскость
class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

// Класс для рендеринга карты
class Render {
public:
    explicit Render(const RenderSettings& settings) : settings_(settings) {}
    
    // Основной метод рендеринга
    void RenderMap(const transport_catalogue::TransportCatalogue& catalogue, std::ostream& out) const;
    
private:
    // Вспомогательные методы
    std::string ColorToString(const Color& color) const;
    void RenderBusLines(const transport_catalogue::TransportCatalogue& catalogue, 
                       const SphereProjector& projector, svg::Document& doc) const;
    void RenderBusLabels(const transport_catalogue::TransportCatalogue& catalogue,
                        const SphereProjector& projector, svg::Document& doc) const;
    void RenderStopSymbols(const transport_catalogue::TransportCatalogue& catalogue,
                          const SphereProjector& projector, svg::Document& doc) const;
    void RenderStopLabels(const transport_catalogue::TransportCatalogue& catalogue,
                         const SphereProjector& projector, svg::Document& doc) const;
    
private:
    RenderSettings settings_;
};

} // namespace map_renderer