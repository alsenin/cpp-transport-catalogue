#include <iostream>
#include <cassert>
#include "test_json.h"
#include "test_transport_catalogue.h"
#include "test_map_renderer.h"
#include "test_request_handler.h"

int main() {
    std::cout << "Running unit tests..." << std::endl;
    
    // Тесты JSON
    std::cout << "\n=== JSON Tests ===" << std::endl;
    test_json::RunAllTests();
    
    // Тесты Transport Catalogue
    std::cout << "\n=== Transport Catalogue Tests ===" << std::endl;
    test_transport_catalogue::RunAllTests();
    
    // Тесты Map Renderer
    std::cout << "\n=== Map Renderer Tests ===" << std::endl;
    test_map_renderer::RunAllTests();
    
    // Тесты Request Handler
    std::cout << "\n=== Request Handler Tests ===" << std::endl;
    test_request_handler::RunAllTests();
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
} 