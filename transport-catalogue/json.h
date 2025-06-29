#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>

using namespace std::literals;

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    Node() = default;
    Node(std::nullptr_t) : value_(nullptr) {}
    Node(int value) : value_(value) {}
    Node(double value) : value_(value) {}
    Node(bool value) : value_(value) {}
    Node(const char* value) : value_(std::string(value)) {}
    Node(std::string value) : value_(std::move(value)) {}
    Node(Array array) : value_(std::move(array)) {}
    Node(Dict map) : value_(std::move(map)) {}
//    Node(std::initializer_list<Node> arr);

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const std::string& AsString() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsDict() const;
    bool IsMap() const { return IsDict(); }

    const Value& GetValue() const { return value_; }

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }
    
    // Перегрузка для bool
    void PrintValue(bool value, std::ostream& out) {
        out << (value ? "true"sv : "false"sv);
    }
    
    // Перегрузка для string
    void PrintValue(const std::string& value, std::ostream& out) {
        out << '"';
        for (char c : value) {
            switch (c) {
                case '"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\r': out << "\\r"; break;
                case '\n': out << "\\n"; break;
                case '\t': out << "\\t"; break;
                default: out << c; break;
            }
        }
        out << '"';
    }
    
    // Перегрузка для Array
    void PrintValue(const Array& value, std::ostream& out) {
        out << '[';
        bool first = true;
        for (const auto& item : value) {
            if (!first) out << ", ";
            PrintNode(item, out);
            first = false;
        }
        out << ']';
    }
    
    // Перегрузка для Dict
    void PrintValue(const Dict& value, std::ostream& out) {
        out << '{';
        bool first = true;
        for (const auto& [key, val] : value) {
            if (!first) out << ", ";
            PrintValue(key, out);
            out << ": ";
            PrintNode(val, out);
            first = false;
        }
        out << '}';
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [this, &out](const auto& value){ PrintValue(value, out); },
            node.GetValue());
    }

    bool operator == (Node& other) const {
        return value_ == other.value_;
    }
    bool operator != (Node& other) const {
        return value_ != other.value_;
    }

    bool operator == (const Node& other) const {
        return value_ == other.value_;
    }
    bool operator != (const Node& other) const {
        return value_ != other.value_;
    }

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator == (Document& other) const {
        return root_ == other.root_;
    }
    
    bool operator != (Document& other) const {
        return root_ != other.root_;
    }    

    bool operator == (const Document& other) const {
        return root_ == other.root_;
    }
    
    bool operator != (const Document& other) const {
        return root_ != other.root_;
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

// Утилиты для работы с Dict/Node
std::string GetStringValue(const Dict& dict, const std::string& field_name);
int GetIntValue(const Dict& dict, const std::string& field_name);
double GetDoubleValue(const Dict& dict, const std::string& field_name);

Node CreateErrorResponse(int request_id, const std::string& error_message);
Node CreateSuccessResponse(int request_id, const Dict& data);

}  // namespace json