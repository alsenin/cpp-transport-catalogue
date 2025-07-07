#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>

int main() {

    transport_catalogue::TransportCatalogue catalogue;
    request_handler::RequestHandler handler(catalogue);
    json_reader::JsonReader reader(catalogue);

    try {
        // Загружаем JSON документ
        json::Document document = reader.LoadDocument(std::cin);

        // Обрабатываем документ для загрузки данных
        handler.ProcessDocument(document);

        // Обрабатываем запросы
        handler.ProcessRequests(document);

    } catch (const json::ParsingError& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
