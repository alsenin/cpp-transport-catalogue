#include "request_handler.h"

#ifdef DEBUG_OUTPUT_REQUEST
#define DEBUG_PRINT(x) std::cerr << "[DEBUG][REQUEST_HANDLER] " << x << std::endl
#else
#define DEBUG_PRINT(x) do {} while(0)
#endif

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace request_handler {

// Реализация RequestRegistry
void RequestRegistry::Register(const std::string& type, RequestCreator creator) {
    creators_[type] = creator;
}

std::unique_ptr<Request> RequestRegistry::Create(const std::string& type, const json::Dict& request_dict, const map_renderer::Render& renderer) const {
    auto it = creators_.find(type);
    if (it == creators_.end()) {
        throw json::ParsingError("Unknown request type: " + type);
    }
    return it->second(request_dict, renderer);
}

// Реализация RequestFactory
std::unique_ptr<Request> RequestFactory::CreateStopRequest(const json::Dict& request_dict, const map_renderer::Render& renderer) {
    (void)renderer; // подавляем предупреждение о неиспользуемом параметре
    std::string name = json::GetStringValue(request_dict, "name");
    int id = json::GetIntValue(request_dict, "id");
    return std::make_unique<StopRequest>(name, id);
}

std::unique_ptr<Request> RequestFactory::CreateBusRequest(const json::Dict& request_dict, const map_renderer::Render& renderer) {
    (void)renderer; // подавляем предупреждение о неиспользуемом параметре
    std::string name = json::GetStringValue(request_dict, "name");
    int id = json::GetIntValue(request_dict, "id");
    return std::make_unique<BusRequest>(name, id);
}

std::unique_ptr<Request> RequestFactory::CreateMapRequest(const json::Dict& request_dict, const map_renderer::Render& renderer) {
    int id = json::GetIntValue(request_dict, "id");
    return std::make_unique<MapRequest>(id, renderer);
}

// Реализация конкретных запросов
json::Node StopRequest::Execute(const transport_catalogue::TransportCatalogue& catalogue) const {
    DEBUG_PRINT("Executing Stop request for: " << name_ << " (id: " << id_ << ")");
    
    const Stop* stop = catalogue.GetStopByName(name_);
    
    if (!stop) {
        return json::CreateErrorResponse(id_, "not found");
    }
    
    std::vector<std::string> buses = catalogue.GetStopInfo(name_);
    
    json::Dict data;
    if (!buses.empty()) {
        json::Array buses_array;
        for (const std::string& bus_name : buses) {
            buses_array.push_back(json::Node(bus_name));
        }
        data["buses"] = json::Node(buses_array);
    } else {
        data["buses"] = json::Node(json::Array{});
    }
    
    return json::CreateSuccessResponse(id_, data);
}

json::Node BusRequest::Execute(const transport_catalogue::TransportCatalogue& catalogue) const {
    DEBUG_PRINT("Executing Bus request for: " << name_ << " (id: " << id_ << ")");
    
    if (!catalogue.RouteExists(name_)) {
        return json::CreateErrorResponse(id_, "not found");
    }
    
    transport_catalogue::RouteInfo route_info = catalogue.GetRouteInfo(name_);
    
    json::Dict data;
    data["route_length"] = json::Node(static_cast<int>(route_info.route_length));
    data["curvature"] = json::Node(route_info.curvature);
    data["stop_count"] = json::Node(route_info.stops_count);
    data["unique_stop_count"] = json::Node(route_info.unique_stops_count);
    
    return json::CreateSuccessResponse(id_, data);
}

json::Node MapRequest::Execute(const transport_catalogue::TransportCatalogue& catalogue) const {
    DEBUG_PRINT("Executing Map request (id: " << id_ << ")");
    
    std::ostringstream svg_stream;
    renderer_.RenderMap(catalogue, svg_stream);
    std::string svg_content = svg_stream.str();
    
    json::Dict data;
    data["map"] = json::Node(svg_content);
    
    return json::CreateSuccessResponse(id_, data);
}

// Реализация RequestHandler
RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue)
    , json_reader_(catalogue)
    , renderer_(json_reader_.GetRenderSettings()) {
    RegisterRequestTypes();
}

void RequestHandler::ProcessDocument(const json::Document& document) {
    DEBUG_PRINT("Processing document for data loading...");
    json_reader_.ProcessDocument(document);
    
    // Обновляем рендерер с новыми настройками
    renderer_ = map_renderer::Render(json_reader_.GetRenderSettings());
    DEBUG_PRINT("Document processed successfully");
}

json::Document RequestHandler::ProcessRequests(const json::Document& document) {
    DEBUG_PRINT("Processing requests from document...");
    
    const json::Node& root = document.GetRoot();
    if (!root.IsDict()) {
        throw json::ParsingError("Root node must be a dictionary");
    }
    
    const json::Dict& root_dict = root.AsMap();
    
    // Обрабатываем статистические запросы
    auto stat_requests_it = root_dict.find("stat_requests");
    if (stat_requests_it != root_dict.end()) {
        ProcessStatRequests(stat_requests_it->second);
    }
    
    // Возвращаем пустой документ (результаты записываются в output в ProcessStatRequests)
    return json::Document(json::Node(json::Dict{}));
}

void RequestHandler::RegisterRequestTypes() {
    request_registry_.Register("Stop", RequestFactory::CreateStopRequest);
    request_registry_.Register("Bus", RequestFactory::CreateBusRequest);
    request_registry_.Register("Map", RequestFactory::CreateMapRequest);
}

json::Node RequestHandler::ProcessSingleRequest(const json::Dict& request_dict) {
    auto type_it = request_dict.find("type");
    if (type_it == request_dict.end() || !type_it->second.IsString()) {
        throw json::ParsingError("Request must have 'type' field as string");
    }
    
    std::string type = type_it->second.AsString();
    auto request = request_registry_.Create(type, request_dict, renderer_);
    return request->Execute(catalogue_);
}

void RequestHandler::ProcessStatRequests(const json::Node& stat_requests) {
    if (!stat_requests.IsArray()) {
        throw json::ParsingError("stat_requests must be an array");
    }
    
    const json::Array& requests = stat_requests.AsArray();
    json::Array responses;
    
    DEBUG_PRINT("Processing " << requests.size() << " stat requests...");
    
    for (const json::Node& request : requests) {
        if (!request.IsDict()) {
            throw json::ParsingError("Request must be a dictionary");
        }
        
        const json::Dict& request_dict = request.AsMap();
        json::Node response = ProcessSingleRequest(request_dict);
        responses.push_back(response);
    }
    
    // Выводим результат в JSON формате
    json::Document result_doc{json::Node(responses)};
    json::Print(result_doc, std::cout);
}

} // namespace request_handler