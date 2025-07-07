cmake_minimum_required(VERSION 3.10)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем предупреждения компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

# Опции для отладочного вывода
option(DEBUG_OUTPUT_JSON "Enable debug output for JSON module" OFF)
option(DEBUG_OUTPUT_REQUEST "Enable debug output for Request Handler module" OFF)
option(DEBUG_OUTPUT_TRANSPORT "Enable debug output for Transport Catalogue module" OFF)
option(DEBUG_OUTPUT_DOMAIN "Enable debug output for Domain module" OFF)
option(DEBUG_OUTPUT_MAP "Enable debug output for Map Renderer module" OFF)
option(DEBUG_OUTPUT_SVG "Enable debug output for SVG module" OFF)
option(DEBUG_OUTPUT_GEO "Enable debug output for Geo module" OFF)

# Устанавливаем определения компилятора на основе опций
if(DEBUG_OUTPUT_JSON)
    add_compile_definitions(DEBUG_OUTPUT_JSON)
endif()

if(DEBUG_OUTPUT_REQUEST)
    add_compile_definitions(DEBUG_OUTPUT_REQUEST)
endif()

if(DEBUG_OUTPUT_TRANSPORT)
    add_compile_definitions(DEBUG_OUTPUT_TRANSPORT)
endif()

if(DEBUG_OUTPUT_DOMAIN)
    add_compile_definitions(DEBUG_OUTPUT_DOMAIN)
endif()

if(DEBUG_OUTPUT_MAP)
    add_compile_definitions(DEBUG_OUTPUT_MAP)
endif()

if(DEBUG_OUTPUT_SVG)
    add_compile_definitions(DEBUG_OUTPUT_SVG)
endif()

if(DEBUG_OUTPUT_GEO)
    add_compile_definitions(DEBUG_OUTPUT_GEO)
endif()

# Добавляем исходные файлы
set(SOURCES
    transport-catalogue/transport_catalogue.cpp
    transport-catalogue/domain.cpp
    transport-catalogue/json.cpp
    transport-catalogue/json_reader.cpp
    transport-catalogue/json_builder.cpp
    transport-catalogue/request_handler.cpp
    transport-catalogue/geo.cpp
    transport-catalogue/map_renderer.cpp
    transport-catalogue/svg.cpp
    transport-catalogue/main.cpp
)

# Добавляем заголовочные файлы
set(HEADERS
    transport-catalogue/transport_catalogue.h
    transport-catalogue/domain.h
    transport-catalogue/json.h
    transport-catalogue/json_reader.h
    transport-catalogue/json_builder.h
    transport-catalogue/request_handler.h
    transport-catalogue/geo.h
    transport-catalogue/map_renderer.h
    transport-catalogue/svg.h
)

# Добавляем include директории
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/transport-catalogue)

# Создаем исполняемый файл
add_executable(transport_catalogue ${SOURCES} ${HEADERS})

# Добавляем директорию с заголовочными файлами
target_include_directories(transport_catalogue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/transport-catalogue)

# Выводим информацию о конфигурации
message(STATUS "=== Debug Output Configuration ===")
message(STATUS "DEBUG_OUTPUT_JSON: ${DEBUG_OUTPUT_JSON}")
message(STATUS "DEBUG_OUTPUT_REQUEST: ${DEBUG_OUTPUT_REQUEST}")
message(STATUS "DEBUG_OUTPUT_TRANSPORT: ${DEBUG_OUTPUT_TRANSPORT}")
message(STATUS "DEBUG_OUTPUT_DOMAIN: ${DEBUG_OUTPUT_DOMAIN}")
message(STATUS "DEBUG_OUTPUT_MAP: ${DEBUG_OUTPUT_MAP}")
message(STATUS "DEBUG_OUTPUT_SVG: ${DEBUG_OUTPUT_SVG}")
message(STATUS "DEBUG_OUTPUT_GEO: ${DEBUG_OUTPUT_GEO}")
message(STATUS "==================================") 