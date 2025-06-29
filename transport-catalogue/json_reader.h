#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace json_reader {

class JsonReader {
public:
    explicit JsonReader(transport_catalogue::TransportCatalogue& catalogue);
    
    // Загрузка JSON документа из потока
    json::Document LoadDocument(std::istream& input);
    
    // Загрузка JSON документа из строки
    json::Document LoadDocument(const std::string& json_string);
    
    // Обработка JSON документа для заполнения каталога
    void ProcessDocument(const json::Document& document);
    
    // Получение настроек рендеринга
    const map_renderer::RenderSettings& GetRenderSettings() const;

private:
    // Основные методы обработки данных
    void ProcessBaseRequestsOptimized(const json::Node& base_requests);
    
    // Методы для парсинга настроек рендеринга
    map_renderer::RenderSettings ParseRenderSettings(const json::Node& render_settings_node);
    map_renderer::Color ParseColor(const json::Node& color_node);
    map_renderer::Offset ParseOffset(const json::Node& offset_node);
    
    // Вспомогательные методы для получения значений из JSON узлов
    std::string GetStringValue(const json::Node& node, const std::string& field_name);
    int GetIntValue(const json::Node& node, const std::string& field_name);
    double GetDoubleValue(const json::Node& node, const std::string& field_name);
    
private:
    transport_catalogue::TransportCatalogue& catalogue_;
    map_renderer::RenderSettings render_settings_;
};

} // namespace json_reader 