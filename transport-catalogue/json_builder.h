#pragma once

#include "json.h"
#include <stdexcept>
#include <string>

namespace json
{

    // Состояния билдера
    enum class BuilderState
    {
        EMPTY,                 // Начальное состояние
        DICT_EXPECTING_KEY,    // В словаре, ожидаем Key
        DICT_EXPECTING_VALUE,  // В словаре, ожидаем Value/StartArray/StartDict после Key
        ARRAY_EXPECTING_VALUE, // В массиве, ожидаем Value/StartArray/StartDict
        READY_TO_BUILD         // Готов к сборке
    };

    class Builder
    {
    public:
        Builder();

        // Forward declarations
        class BaseContext;
        class DictContext;
        class KeyContext;
        class ValueContext;
        class ArrayContext;

        // Базовый класс для всех контекстов
        class BaseContext
        {
        protected:
            explicit BaseContext(Builder &builder) : builder_(builder) {}
            Builder &builder_;
        };

        // Контекст для словаря - можно только добавлять ключи и завершать словарь
        class DictContext : public BaseContext
        {
        public:
            explicit DictContext(Builder &builder) : BaseContext(builder) {}

            // Разрешенные методы - возвращают следующий контекст
            KeyContext Key(std::string key);
            Builder &EndDict();
        };

        // Контекст для ключа - можно только добавлять значение
        class KeyContext : public BaseContext
        {
        public:
            explicit KeyContext(Builder &builder) : BaseContext(builder) {}

            // Разрешенные методы
            ValueContext Value(json::Node value);
            ValueContext Value(std::string value);
            ValueContext Value(int value);
            ValueContext Value(double value);
            ValueContext Value(bool value);
            DictContext StartDict();
            ArrayContext StartArray();
        };

        // Контекст для значения - можно добавлять ключи (возврат к DictContext)
        class ValueContext : public BaseContext
        {
        public:
            explicit ValueContext(Builder &builder) : BaseContext(builder) {}

            // Разрешенные методы - после Value можно только Key или EndDict
            KeyContext Key(std::string key);
            Builder &EndDict();
        };

        // Контекст для массива - можно добавлять значения и завершать массив
        class ArrayContext : public BaseContext
        {
        public:
            explicit ArrayContext(Builder &builder) : BaseContext(builder) {}

            // Разрешенные методы
            ArrayContext &Value(json::Node value);
            ArrayContext &Value(std::string value);
            ArrayContext &Value(int value);
            ArrayContext &Value(double value);
            ArrayContext &Value(bool value);
            DictContext StartDict();
            ArrayContext StartArray();
            Builder &EndArray();
        };

        // Методы для начала построения - возвращают контексты
        DictContext StartDict();
        ArrayContext StartArray();

        // Методы для простых значений - возвращают Builder для завершения
        Builder &Value(json::Node value);
        Builder &Value(std::string value);
        Builder &Value(int value);
        Builder &Value(double value);
        Builder &Value(bool value);

        // Внутренние методы для контекстов
        Builder &Key(std::string key);
        Builder &EndDict();
        Builder &EndArray();

        json::Node Build();
        ~Builder();

    private:
        json::Node root_;
        std::vector<json::Node *> nodes_stack_;
        std::string current_key_;
        BuilderState state_;
        void CheckState(BuilderState expected, const std::string &operation);
        void TransitionTo(BuilderState new_state);
        void AddNode(json::Node node);
    };

} // namespace json
