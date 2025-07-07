#include "json_reader.h"

#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#endif
#define DEBUG_PRINT(x) \
    do                 \
    {                  \
    } while (0)

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace json_reader
{

    JsonReader::JsonReader(transport_catalogue::TransportCatalogue &catalogue)
        : catalogue_(catalogue)
    {
    }

    json::Document JsonReader::LoadDocument(std::istream &input)
    {
        try
        {
            return json::Load(input);
        }
        catch (const json::ParsingError &e)
        {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            throw;
        }
    }

    json::Document JsonReader::LoadDocument(const std::string &json_string)
    {
        std::istringstream input(json_string);
        return LoadDocument(input);
    }

    void JsonReader::ProcessDocument(const json::Document &document)
    {
        const json::Node &root = document.GetRoot();

        if (!root.IsDict())
        {
            throw json::ParsingError("Root node must be a dictionary");
        }

        const json::Dict &root_dict = root.AsMap();

        // Обрабатываем базовые запросы (остановки и маршруты)
        auto base_requests_it = root_dict.find("base_requests");
        if (base_requests_it != root_dict.end())
        {
            ProcessBaseRequestsOptimized(base_requests_it->second);
        }

        // Обрабатываем настройки рендеринга
        auto render_settings_it = root_dict.find("render_settings");
        if (render_settings_it != root_dict.end())
        {
            render_settings_ = ParseRenderSettings(render_settings_it->second);
            DEBUG_PRINT("Parsed render settings successfully");
        }
    }

    void JsonReader::ProcessBaseRequestsOptimized(const json::Node &base_requests)
    {
        if (!base_requests.IsArray())
        {
            throw json::ParsingError("base_requests must be an array");
        }

        const json::Array &requests = base_requests.AsArray();

        // Временные массивы для сбора данных
        std::vector<std::pair<std::string, std::pair<double, double>>> stops;
        std::vector<std::tuple<std::string, std::string, double>> distances;
        std::vector<std::pair<std::string, std::vector<std::string>>> buses;
        std::vector<bool> is_roundtrip; // для маршрутов

        DEBUG_PRINT("Phase 1: Collecting data in single pass...");

        // Один проход по всем запросам
        for (const json::Node &request : requests)
        {
            if (!request.IsDict())
            {
                throw json::ParsingError("Request must be a dictionary");
            }

            const json::Dict &request_dict = request.AsMap();
            auto type_it = request_dict.find("type");

            if (type_it == request_dict.end() || !type_it->second.IsString())
            {
                throw json::ParsingError("Request must have 'type' field as string");
            }

            std::string type = type_it->second.AsString();

            if (type == "Stop")
            {
                // Собираем данные об остановке
                std::string stop_name = GetStringValue(request_dict, "name");
                double latitude = GetDoubleValue(request_dict, "latitude");
                double longitude = GetDoubleValue(request_dict, "longitude");
                stops.emplace_back(stop_name, std::make_pair(latitude, longitude));
                DEBUG_PRINT("Collected stop: " << stop_name << " at (" << latitude << ", " << longitude << ")");

                // Собираем расстояния для этой остановки
                auto distances_it = request_dict.find("road_distances");
                if (distances_it != request_dict.end())
                {
                    if (!distances_it->second.IsDict())
                    {
                        throw json::ParsingError("road_distances must be a dictionary");
                    }

                    const json::Dict &road_distances = distances_it->second.AsMap();
                    for (const auto &[target_stop, distance_node] : road_distances)
                    {
                        if (!distance_node.IsDouble())
                        {
                            throw json::ParsingError("Distance must be a number");
                        }
                        distances.emplace_back(stop_name, target_stop, distance_node.AsDouble());
                        DEBUG_PRINT("Collected distance: " << stop_name << " -> " << target_stop << " = " << distance_node.AsDouble());
                    }
                }
            }
            else if (type == "Bus")
            {
                // Собираем данные о маршруте
                std::string bus_name = GetStringValue(request_dict, "name");
                auto stops_it = request_dict.find("stops");
                if (stops_it == request_dict.end() || !stops_it->second.IsArray())
                {
                    throw json::ParsingError("Bus stops must be an array");
                }
                const json::Array &stops_array = stops_it->second.AsArray();
                std::vector<std::string> stop_names;
                for (const json::Node &stop_node : stops_array)
                {
                    if (!stop_node.IsString())
                    {
                        throw json::ParsingError("Stop name must be a string");
                    }
                    stop_names.push_back(stop_node.AsString());
                }
                buses.emplace_back(bus_name, stop_names);
                // Проверяем, является ли маршрут кольцевым
                bool roundtrip = false;
                auto roundtrip_it = request_dict.find("is_roundtrip");
                if (roundtrip_it != request_dict.end())
                {
                    if (!roundtrip_it->second.IsBool())
                    {
                        throw json::ParsingError("is_roundtrip must be a boolean");
                    }
                    roundtrip = roundtrip_it->second.AsBool();
                }
                is_roundtrip.push_back(roundtrip);
                DEBUG_PRINT("Collected bus route: " << bus_name << " with " << stop_names.size() << " stops, roundtrip: " << roundtrip);
            }
            else
            {
                throw json::ParsingError("Unknown request type: " + type);
            }
        }

        // Фаза 2: Пакетное добавление данных в каталог
        DEBUG_PRINT("Phase 2: Bulk adding " << stops.size() << " stops...");
        catalogue_.AddStops(stops);

        DEBUG_PRINT("Phase 3: Bulk adding " << distances.size() << " distances...");
        catalogue_.AddDistances(distances);

        DEBUG_PRINT("Phase 4: Bulk adding " << buses.size() << " routes...");
        // Добавляем маршруты с учетом is_roundtrip
        for (size_t i = 0; i < buses.size(); ++i)
        {
            catalogue_.AddRoute(buses[i].first, buses[i].second, is_roundtrip[i]);
        }
        DEBUG_PRINT("Optimized processing completed successfully!");
    }

    std::string JsonReader::GetStringValue(const json::Node &node, const std::string &field_name)
    {
        if (!node.IsDict())
        {
            throw json::ParsingError("Node must be a dictionary to get field: " + field_name);
        }

        const json::Dict &dict = node.AsMap();
        auto it = dict.find(field_name);

        if (it == dict.end())
        {
            throw json::ParsingError("Field '" + field_name + "' not found");
        }

        if (!it->second.IsString())
        {
            throw json::ParsingError("Field '" + field_name + "' is not a string");
        }

        return it->second.AsString();
    }

    int JsonReader::GetIntValue(const json::Node &node, const std::string &field_name)
    {
        if (!node.IsDict())
        {
            throw json::ParsingError("Node must be a dictionary to get field: " + field_name);
        }

        const json::Dict &dict = node.AsMap();
        auto it = dict.find(field_name);

        if (it == dict.end())
        {
            throw json::ParsingError("Field '" + field_name + "' not found");
        }

        if (!it->second.IsInt())
        {
            throw json::ParsingError("Field '" + field_name + "' is not an integer");
        }

        return it->second.AsInt();
    }

    double JsonReader::GetDoubleValue(const json::Node &node, const std::string &field_name)
    {
        if (!node.IsDict())
        {
            throw json::ParsingError("Node must be a dictionary to get field: " + field_name);
        }

        const json::Dict &dict = node.AsMap();
        auto it = dict.find(field_name);

        if (it == dict.end())
        {
            throw json::ParsingError("Field '" + field_name + "' not found");
        }

        if (!it->second.IsDouble())
        {
            throw json::ParsingError("Field '" + field_name + "' is not a number");
        }

        return it->second.AsDouble();
    }

    const map_renderer::RenderSettings &JsonReader::GetRenderSettings() const
    {
        return render_settings_;
    }

    map_renderer::RenderSettings JsonReader::ParseRenderSettings(const json::Node &render_settings_node)
    {
        if (!render_settings_node.IsDict())
        {
            throw json::ParsingError("render_settings must be a dictionary");
        }

        const json::Dict &settings_dict = render_settings_node.AsMap();
        map_renderer::RenderSettings settings;

        // Парсим width
        auto width_it = settings_dict.find("width");
        if (width_it != settings_dict.end())
        {
            if (!width_it->second.IsDouble())
            {
                throw json::ParsingError("width must be a number");
            }
            double width = width_it->second.AsDouble();
            if (width < 0.0 || width > 100000.0)
            {
                throw json::ParsingError("width must be in range [0, 100000]");
            }
            settings.width = width;
        }

        // Парсим height
        auto height_it = settings_dict.find("height");
        if (height_it != settings_dict.end())
        {
            if (!height_it->second.IsDouble())
            {
                throw json::ParsingError("height must be a number");
            }
            double height = height_it->second.AsDouble();
            if (height < 0.0 || height > 100000.0)
            {
                throw json::ParsingError("height must be in range [0, 100000]");
            }
            settings.height = height;
        }

        // Парсим padding
        auto padding_it = settings_dict.find("padding");
        if (padding_it != settings_dict.end())
        {
            if (!padding_it->second.IsDouble())
            {
                throw json::ParsingError("padding must be a number");
            }
            double padding = padding_it->second.AsDouble();
            double min_dimension = std::min(settings.width, settings.height);
            if (padding < 0.0 || padding >= min_dimension / 2.0)
            {
                throw json::ParsingError("padding must be >= 0 and < min(width, height)/2");
            }
            settings.padding = padding;
        }

        // Парсим line_width
        auto line_width_it = settings_dict.find("line_width");
        if (line_width_it != settings_dict.end())
        {
            if (!line_width_it->second.IsDouble())
            {
                throw json::ParsingError("line_width must be a number");
            }
            double line_width = line_width_it->second.AsDouble();
            if (line_width < 0.0 || line_width > 100000.0)
            {
                throw json::ParsingError("line_width must be in range [0, 100000]");
            }
            settings.line_width = line_width;
        }

        // Парсим stop_radius
        auto stop_radius_it = settings_dict.find("stop_radius");
        if (stop_radius_it != settings_dict.end())
        {
            if (!stop_radius_it->second.IsDouble())
            {
                throw json::ParsingError("stop_radius must be a number");
            }
            double stop_radius = stop_radius_it->second.AsDouble();
            if (stop_radius < 0.0 || stop_radius > 100000.0)
            {
                throw json::ParsingError("stop_radius must be in range [0, 100000]");
            }
            settings.stop_radius = stop_radius;
        }

        // Парсим bus_label_font_size
        auto bus_label_font_size_it = settings_dict.find("bus_label_font_size");
        if (bus_label_font_size_it != settings_dict.end())
        {
            if (!bus_label_font_size_it->second.IsInt())
            {
                throw json::ParsingError("bus_label_font_size must be an integer");
            }
            int bus_label_font_size = bus_label_font_size_it->second.AsInt();
            if (bus_label_font_size < 0 || bus_label_font_size > 100000)
            {
                throw json::ParsingError("bus_label_font_size must be in range [0, 100000]");
            }
            settings.bus_label_font_size = bus_label_font_size;
        }

        // Парсим bus_label_offset
        auto bus_label_offset_it = settings_dict.find("bus_label_offset");
        if (bus_label_offset_it != settings_dict.end())
        {
            settings.bus_label_offset = ParseOffset(bus_label_offset_it->second);
        }

        // Парсим stop_label_font_size
        auto stop_label_font_size_it = settings_dict.find("stop_label_font_size");
        if (stop_label_font_size_it != settings_dict.end())
        {
            if (!stop_label_font_size_it->second.IsInt())
            {
                throw json::ParsingError("stop_label_font_size must be an integer");
            }
            int stop_label_font_size = stop_label_font_size_it->second.AsInt();
            if (stop_label_font_size < 0 || stop_label_font_size > 100000)
            {
                throw json::ParsingError("stop_label_font_size must be in range [0, 100000]");
            }
            settings.stop_label_font_size = stop_label_font_size;
        }

        // Парсим stop_label_offset
        auto stop_label_offset_it = settings_dict.find("stop_label_offset");
        if (stop_label_offset_it != settings_dict.end())
        {
            settings.stop_label_offset = ParseOffset(stop_label_offset_it->second);
        }

        // Парсим underlayer_color
        auto underlayer_color_it = settings_dict.find("underlayer_color");
        if (underlayer_color_it != settings_dict.end())
        {
            settings.underlayer_color = ParseColor(underlayer_color_it->second);
        }

        // Парсим underlayer_width
        auto underlayer_width_it = settings_dict.find("underlayer_width");
        if (underlayer_width_it != settings_dict.end())
        {
            if (!underlayer_width_it->second.IsDouble())
            {
                throw json::ParsingError("underlayer_width must be a number");
            }
            double underlayer_width = underlayer_width_it->second.AsDouble();
            if (underlayer_width < 0.0 || underlayer_width > 100000.0)
            {
                throw json::ParsingError("underlayer_width must be in range [0, 100000]");
            }
            settings.underlayer_width = underlayer_width;
        }

        // Парсим color_palette
        auto color_palette_it = settings_dict.find("color_palette");
        if (color_palette_it != settings_dict.end())
        {
            if (!color_palette_it->second.IsArray())
            {
                throw json::ParsingError("color_palette must be an array");
            }
            const json::Array &palette_array = color_palette_it->second.AsArray();
            if (palette_array.empty())
            {
                throw json::ParsingError("color_palette must not be empty");
            }

            for (const json::Node &color_node : palette_array)
            {
                settings.color_palette.push_back(ParseColor(color_node));
            }
        }

        return settings;
    }

    map_renderer::Color JsonReader::ParseColor(const json::Node &color_node)
    {
        if (color_node.IsString())
        {
            // Цвет задан строкой (например, "red", "black")
            return map_renderer::Color(color_node.AsString());
        }
        else if (color_node.IsArray())
        {
            const json::Array &color_array = color_node.AsArray();

            if (color_array.size() == 3)
            {
                // RGB формат [r, g, b]
                if (!color_array[0].IsInt() || !color_array[1].IsInt() || !color_array[2].IsInt())
                {
                    throw json::ParsingError("RGB color components must be integers");
                }
                int r = color_array[0].AsInt();
                int g = color_array[1].AsInt();
                int b = color_array[2].AsInt();

                if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
                {
                    throw json::ParsingError("RGB color components must be in range [0, 255]");
                }

                return map_renderer::Color(r, g, b);
            }
            else if (color_array.size() == 4)
            {
                // RGBA формат [r, g, b, opacity]
                if (!color_array[0].IsInt() || !color_array[1].IsInt() ||
                    !color_array[2].IsInt() || !color_array[3].IsDouble())
                {
                    throw json::ParsingError("RGBA color components must be [int, int, int, double]");
                }
                int r = color_array[0].AsInt();
                int g = color_array[1].AsInt();
                int b = color_array[2].AsInt();
                double opacity = color_array[3].AsDouble();

                if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
                {
                    throw json::ParsingError("RGB color components must be in range [0, 255]");
                }
                if (opacity < 0.0 || opacity > 1.0)
                {
                    throw json::ParsingError("Opacity must be in range [0.0, 1.0]");
                }

                return map_renderer::Color(r, g, b, opacity);
            }
            else
            {
                throw json::ParsingError("Color array must have 3 (RGB) or 4 (RGBA) elements");
            }
        }
        else
        {
            throw json::ParsingError("Color must be a string or array");
        }
    }

    map_renderer::Offset JsonReader::ParseOffset(const json::Node &offset_node)
    {
        if (!offset_node.IsArray())
        {
            throw json::ParsingError("Offset must be an array");
        }

        const json::Array &offset_array = offset_node.AsArray();
        if (offset_array.size() != 2)
        {
            throw json::ParsingError("Offset must be an array with exactly 2 elements");
        }

        if (!offset_array[0].IsDouble() || !offset_array[1].IsDouble())
        {
            throw json::ParsingError("Offset elements must be numbers");
        }

        double dx = offset_array[0].AsDouble();
        double dy = offset_array[1].AsDouble();

        if (dx < -100000.0 || dx > 100000.0 || dy < -100000.0 || dy > 100000.0)
        {
            throw json::ParsingError("Offset values must be in range [-100000, 100000]");
        }

        return map_renderer::Offset(dx, dy);
    }

} // namespace json_reader
