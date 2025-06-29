# Transport Catalogue

Система управления транспортным каталогом с поддержкой JSON-запросов и визуализацией карт маршрутов в формате SVG.

## Архитектура проекта

### Структура директорий
```
cpp-transport-catalogue/
├── CMakeLists.txt              # Конфигурация сборки CMake
├── transport-catalogue/        # Основной код проекта
│   ├── transport_catalogue.h/cpp  # Главный класс каталога
│   ├── domain.h/cpp              # Слой предметной области
│   ├── json.h/cpp                # JSON обработка
│   ├── json_reader.h/cpp         # JSON парсер
│   ├── request_handler.h/cpp     # Обработка запросов
│   ├── map_renderer.h/cpp        # Рендеринг карт
│   ├── svg.h/cpp                 # SVG библиотека
│   ├── geo.h/cpp                 # Географические утилиты
│   └── main.cpp                  # Точка входа
├── tests/                       # Тесты
│   ├── test_main.cpp
│   ├── test_transport_catalogue.h/cpp
│   ├── test_json.h/cpp
│   ├── test_map_renderer.h/cpp
│   └── test_request_handler.h/cpp
└── README.md                    # Этот файл
```

### Основные компоненты

#### 1. **Transport Catalogue** (`transport_catalogue.h/cpp`)
Центральный компонент системы, управляющий данными об остановках и маршрутах.

**Основные классы:**
- `TransportCatalogue` - главный класс каталога
- `RouteInfo` - структура с информацией о маршруте

**Ключевые методы:**
- `AddStops()` - добавление остановок
- `AddRoute()` - добавление маршрутов
- `AddDistances()` - добавление расстояний между остановками
- `GetRouteInfo()` - получение информации о маршруте
- `GetStopInfo()` - получение информации об остановке

#### 2. **Domain Layer** (`domain.h`)
Слой предметной области, содержащий базовые структуры данных.

**Структуры:**
- `Stop` - информация об остановке (название, координаты)
- `Route` - информация о маршруте (название, остановки, тип)

**Контейнеры:**
- `Container<T>` - базовый шаблонный контейнер
- `StopContainer` - контейнер для остановок
- `RouteContainer` - контейнер для маршрутов

#### 3. **JSON Processing** (`json.h/cpp`, `json_reader.h/cpp`)
Обработка JSON-данных и парсинг запросов.

**Основные классы:**
- `json::Node` - узел JSON-дерева
- `json::Document` - JSON-документ
- `JsonReader` - парсер JSON-запросов

#### 4. **Request Handler** (`request_handler.h/cpp`)
Обработка различных типов запросов.

**Архитектура запросов:**
- `Request` - базовый класс запроса
- `StopRequest` - запрос информации об остановке
- `BusRequest` - запрос информации о маршруте
- `MapRequest` - запрос на генерацию карты
- `RequestFactory` - фабрика для создания запросов
- `RequestRegistry` - реестр типов запросов

#### 5. **Map Renderer** (`map_renderer.h/cpp`)
Визуализация карт маршрутов в формате SVG.

**Основные компоненты:**
- `Render` - основной класс рендерера
- `RenderSettings` - настройки отображения
- `SphereProjector` - проекция географических координат

#### 6. **SVG Library** (`svg.h/cpp`)
Библиотека для работы с SVG-элементами.

**Элементы:**
- `Document` - SVG-документ
- `Polyline` - полилиния (маршрут)
- `Text` - текст (названия остановок/маршрутов)
- `Circle` - круг (символ остановки)

#### 7. **Geographic Utilities** (`geo.h/cpp`)
Утилиты для работы с географическими координатами.

**Функции:**
- `ComputeDistance()` - вычисление расстояния между точками
- `Coordinates` - структура координат

## Реализация кэширования в Transport Catalogue

### Механизм кэширования

Кэширование реализовано для оптимизации поиска маршрутов по остановкам. Кэш хранит соответствие "остановка → список маршрутов".

#### Принцип работы:

1. **Инициализация кэша:**
   - При первом обращении к `GetStopInfo()` вызывается `UpdateCache()`
   - Проходим по всем маршрутам и для каждой остановки добавляем название маршрута

2. **Инвалидация кэша:**
   - При добавлении новых остановок или маршрутов вызывается `InvalidateCache()`
   - Устанавливается `cache_valid_ = false`

3. **Обновление кэша:**
   - Если кэш недействителен, пересчитываем его заново
   - Удаляем дубликаты маршрутов для каждой остановки
   - Сортируем маршруты по алфавиту

#### Оптимизации:
- **Ленивая инициализация** - кэш создается только при необходимости
- **Автоматическая инвалидация** - при изменении данных кэш автоматически помечается как недействительный
- **Дублирование маршрутов** - автоматическое удаление дубликатов
- **Сортировка** - маршруты сортируются для консистентности

#### Оценки сложности операций:

**Без кэширования:**
- `GetStopInfo()`: O(R × S), где R - количество маршрутов, S - среднее количество остановок в маршруте
- `AddStop()`: O(1)
- `AddRoute()`: O(S), где S - количество остановок в маршруте
- `GetRouteInfo()`: O(S)

**С кэшированием:**
- `GetStopInfo()`: O(1) - прямой доступ к кэшу
- `AddStop()`: O(1) + инвалидация кэша
- `AddRoute()`: O(S) + инвалидация кэша
- `GetRouteInfo()`: O(S) - без изменений
- `UpdateCache()`: O(R × S) - выполняется только при необходимости

**Пространственная сложность:**
- Кэш: O(S × R), где S - количество остановок, R - среднее количество маршрутов на остановку
- Общая память: O(S + R + S×R)

### Пример использования кэша:
```cpp
std::vector<std::string> TransportCatalogue::GetStopInfo(const std::string& stop_name) const {
    UpdateCache(); // Обновляем кэш при необходимости
    
    auto it = stop_to_routes_cache_.find(stop_name);
    if (it != stop_to_routes_cache_.end()) {
        return it->second; // Возвращаем кэшированный результат
    }
    return {}; // Остановка не найдена
}
```

## Сборка и тестирование

### Требования
- CMake 3.10+
- C++17 совместимый компилятор (GCC, Clang, MSVC)

### Сборка
```bash
# Генерация файлов сборки
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Сборка основного приложения
cmake --build build --target transport_catalogue

# Сборка тестов
cmake --build build --target transport_catalogue_tests
```

### Запуск тестов
```bash
# Все тесты
./build/transport_catalogue_tests

# С отладочным выводом
cmake -B build -DDEBUG_OUTPUT_TRANSPORT=ON -DDEBUG_OUTPUT_DOMAIN=ON
cmake --build build --target transport_catalogue_tests
./build/transport_catalogue_tests
```

### Отладочный вывод

Проект поддерживает отладочный вывод для каждого модуля:

- `DEBUG_OUTPUT_JSON` - отладка JSON модуля
- `DEBUG_OUTPUT_REQUEST` - отладка Request Handler
- `DEBUG_OUTPUT_TRANSPORT` - отладка Transport Catalogue
- `DEBUG_OUTPUT_DOMAIN` - отладка Domain модуля
- `DEBUG_OUTPUT_MAP` - отладка Map Renderer
- `DEBUG_OUTPUT_SVG` - отладка SVG модуля
- `DEBUG_OUTPUT_GEO` - отладка Geo модуля

Пример отладочного вывода:
```
[DEBUG][TRANSPORT] AddStops: adding 3 stops
[DEBUG][TRANSPORT] Adding stop: Stop1 (55.6111, 37.2083)
[DEBUG][TRANSPORT] AddRoute: Bus1 with 3 stops, roundtrip: 0
[DEBUG][TRANSPORT] Cache is valid, skipping update
[DEBUG][TRANSPORT] Found 2 routes for stop 'Stop1'
```

## Использование

### Формат входных данных

Проект принимает JSON-запросы для добавления данных и получения информации:

#### Добавление остановок:
```json
{
  "type": "Stop",
  "name": "Stop1",
  "latitude": 55.611087,
  "longitude": 37.20829
}
```

#### Добавление маршрутов:
```json
{
  "type": "Bus",
  "name": "Bus1",
  "stops": ["Stop1", "Stop2", "Stop3"],
  "is_roundtrip": false
}
```

#### Запрос информации:
```json
{
  "id": 1,
  "type": "Bus",
  "name": "Bus1"
}
```

### Примеры использования

#### Получение информации о маршруте:
```json
{"id": 1, "type": "Bus", "name": "Bus1"}
```

**Ответ:**
```json
{
  "route_length": 5000,
  "request_id": 1,
  "curvature": 0.238783,
  "stop_count": 5,
  "unique_stop_count": 3
}
```

#### Получение информации об остановке:
```json
{"id": 2, "type": "Stop", "name": "Stop1"}
```

**Ответ:**
```json
{
  "buses": ["Bus1", "Bus2"],
  "request_id": 2
}
```

#### Генерация карты:
```json
{"id": 3, "type": "Map"}
```

**Ответ:**
```json
{
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">...</svg>",
  "request_id": 3
}
```

## Лицензия

Проект разработан в рамках образовательной программы.

