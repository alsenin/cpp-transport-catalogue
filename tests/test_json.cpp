#include "test_json.h"
#include <iostream>
#include <cassert>
#include <sstream>

namespace test_json {

void TestNodeCreation() {
    std::cout << "  TestNodeCreation... ";
    
    // Тест создания пустого узла
    json::Node empty_node;
    assert(empty_node.IsNull());
    
    // Тест создания узлов разных типов
    json::Node int_node(42);
    assert(int_node.IsInt());
    assert(int_node.AsInt() == 42);
    
    json::Node double_node(3.14);
    assert(double_node.IsDouble());
    assert(double_node.AsDouble() == 3.14);
    
    json::Node string_node("hello");
    assert(string_node.IsString());
    assert(string_node.AsString() == "hello");
    
    json::Node bool_node(true);
    assert(bool_node.IsBool());
    assert(bool_node.AsBool() == true);
    
    std::cout << "PASSED" << std::endl;
}

void TestNodeTypes() {
    std::cout << "  TestNodeTypes... ";
    
    json::Node int_node(42);
    assert(int_node.IsInt());
    assert(int_node.IsDouble()); // int тоже является double
    assert(!int_node.IsPureDouble());
    
    json::Node double_node(3.14);
    assert(double_node.IsDouble());
    assert(double_node.IsPureDouble());
    assert(!double_node.IsInt());
    
    json::Node string_node("test");
    assert(string_node.IsString());
    assert(!string_node.IsInt());
    assert(!string_node.IsDouble());
    
    json::Node bool_node(false);
    assert(bool_node.IsBool());
    assert(!bool_node.IsString());
    
    json::Node null_node;
    assert(null_node.IsNull());
    assert(!null_node.IsInt());
    
    std::cout << "PASSED" << std::endl;
}

void TestNodeAccess() {
    std::cout << "  TestNodeAccess... ";
    
    json::Node int_node(42);
    assert(int_node.AsInt() == 42);
    
    json::Node double_node(3.14);
    assert(double_node.AsDouble() == 3.14);
    
    json::Node string_node("hello world");
    assert(string_node.AsString() == "hello world");
    
    json::Node bool_node(true);
    assert(bool_node.AsBool() == true);
    
    // Тест доступа к массиву
    json::Array arr = {json::Node(1), json::Node(2), json::Node(3)};
    json::Node array_node(arr);
    assert(array_node.IsArray());
    assert(array_node.AsArray().size() == 3);
    assert(array_node.AsArray()[0].AsInt() == 1);
    
    // Тест доступа к словарю
    json::Dict dict = {{"key1", json::Node("value1")}, {"key2", json::Node(42)}};
    json::Node dict_node(dict);
    assert(dict_node.IsDict());
    assert(dict_node.AsMap().size() == 2);
    assert(dict_node.AsMap().at("key1").AsString() == "value1");
    assert(dict_node.AsMap().at("key2").AsInt() == 42);
    
    std::cout << "PASSED" << std::endl;
}

void TestArrayOperations() {
    std::cout << "  TestArrayOperations... ";
    
    json::Array arr;
    arr.push_back(json::Node(1));
    arr.push_back(json::Node("hello"));
    arr.push_back(json::Node(true));
    
    assert(arr.size() == 3);
    assert(arr[0].AsInt() == 1);
    assert(arr[1].AsString() == "hello");
    assert(arr[2].AsBool() == true);
    
    json::Node array_node(arr);
    assert(array_node.IsArray());
    assert(array_node.AsArray().size() == 3);
    
    std::cout << "PASSED" << std::endl;
}

void TestDictOperations() {
    std::cout << "  TestDictOperations... ";
    
    json::Dict dict;
    dict["int"] = json::Node(42);
    dict["string"] = json::Node("test");
    dict["bool"] = json::Node(false);
    
    assert(dict.size() == 3);
    assert(dict["int"].AsInt() == 42);
    assert(dict["string"].AsString() == "test");
    assert(dict["bool"].AsBool() == false);
    
    json::Node dict_node(dict);
    assert(dict_node.IsDict());
    assert(dict_node.AsMap().size() == 3);
    
    std::cout << "PASSED" << std::endl;
}

void TestJsonParsing() {
    std::cout << "  TestJsonParsing... ";
    
    // Тест парсинга простых типов
    std::istringstream iss1("42");
    auto int_doc = json::Load(iss1);
    assert(int_doc.GetRoot().IsInt());
    assert(int_doc.GetRoot().AsInt() == 42);
    
    std::istringstream iss2("\"hello\"");
    auto string_doc = json::Load(iss2);
    assert(string_doc.GetRoot().IsString());
    assert(string_doc.GetRoot().AsString() == "hello");
    
    std::istringstream iss3("true");
    auto bool_doc = json::Load(iss3);
    assert(bool_doc.GetRoot().IsBool());
    assert(bool_doc.GetRoot().AsBool() == true);
    
    std::istringstream iss4("null");
    auto null_doc = json::Load(iss4);
    assert(null_doc.GetRoot().IsNull());
    
    // Тест парсинга массива
    std::istringstream iss5("[1, 2, 3]");
    auto array_doc = json::Load(iss5);
    assert(array_doc.GetRoot().IsArray());
    auto arr = array_doc.GetRoot().AsArray();
    assert(arr.size() == 3);
    assert(arr[0].AsInt() == 1);
    assert(arr[1].AsInt() == 2);
    assert(arr[2].AsInt() == 3);
    
    // Тест парсинга объекта
    std::istringstream iss6("{\"key\": \"value\", \"num\": 42}");
    auto obj_doc = json::Load(iss6);
    assert(obj_doc.GetRoot().IsDict());
    auto obj = obj_doc.GetRoot().AsMap();
    assert(obj.size() == 2);
    assert(obj["key"].AsString() == "value");
    assert(obj["num"].AsInt() == 42);
    
    std::cout << "PASSED" << std::endl;
}

void TestJsonPrinting() {
    std::cout << "  TestJsonPrinting... ";
    
    // Тест печати простых типов
    json::Node int_node(42);
    std::ostringstream oss1;
    json::Print(json::Document(int_node), oss1);
    assert(oss1.str() == "42");
    
    json::Node string_node("hello");
    std::ostringstream oss2;
    json::Print(json::Document(string_node), oss2);
    assert(oss2.str() == "\"hello\"");
    
    json::Node bool_node(true);
    std::ostringstream oss3;
    json::Print(json::Document(bool_node), oss3);
    assert(oss3.str() == "true");
    
    // Тест печати массива
    json::Array arr = {json::Node(1), json::Node(2), json::Node(3)};
    json::Node array_node(arr);
    std::ostringstream oss4;
    json::Print(json::Document(array_node), oss4);
    assert(oss4.str() == "[1, 2, 3]");
    
    // Тест печати объекта
    json::Dict dict = {{"key", json::Node("value")}, {"num", json::Node(42)}};
    json::Node dict_node(dict);
    std::ostringstream oss5;
    json::Print(json::Document(dict_node), oss5);
    assert(oss5.str() == "{\"key\": \"value\", \"num\": 42}");
    
    std::cout << "PASSED" << std::endl;
}

void TestJsonSerialization() {
    std::cout << "  TestJsonSerialization... ";
    
    // Создаем сложный JSON объект
    json::Dict dict;
    dict["string"] = json::Node("hello");
    dict["number"] = json::Node(42);
    dict["boolean"] = json::Node(true);
    dict["null"] = json::Node();
    
    json::Array arr = {json::Node(1), json::Node(2), json::Node(3)};
    dict["array"] = json::Node(arr);
    
    json::Dict nested = {{"nested_key", json::Node("nested_value")}};
    dict["object"] = json::Node(nested);
    
    json::Node root(dict);
    
    // Сериализуем и десериализуем
    std::ostringstream oss;
    json::Print(json::Document(root), oss);
    std::string serialized = oss.str();
    std::istringstream iss(serialized);
    auto parsed = json::Load(iss);
    auto parsed_dict = parsed.GetRoot().AsMap();
    
    assert(parsed_dict.at("string").AsString() == "hello");
    assert(parsed_dict.at("number").AsInt() == 42);
    assert(parsed_dict.at("boolean").AsBool() == true);
    assert(parsed_dict.at("null").IsNull());
    assert(parsed_dict.at("array").AsArray().size() == 3);
    assert(parsed_dict.at("object").AsMap().at("nested_key").AsString() == "nested_value");
    
    std::cout << "PASSED" << std::endl;
}

void TestErrorHandling() {
    std::cout << "  TestErrorHandling... ";
    
    // Тест обработки некорректного JSON
    try {
        std::istringstream iss("invalid json");
        json::Load(iss);
        assert(false); // Не должно дойти до сюда
    } catch (const json::ParsingError&) {
        // Ожидаемое исключение
    }
    
    try {
        std::istringstream iss("[1, 2, 3,");
        json::Load(iss);
        assert(false); // Не должно дойти до сюда
    } catch (const json::ParsingError&) {
        // Ожидаемое исключение
    }
    
    try {
        std::istringstream iss("{\"key\": \"value\",}");
        json::Load(iss);
        assert(false); // Не должно дойти до сюда
    } catch (const json::ParsingError&) {
        // Ожидаемое исключение
    }
    
    std::cout << "PASSED" << std::endl;
}

void RunAllTests() {
    TestNodeCreation();
    TestNodeTypes();
    TestNodeAccess();
    TestArrayOperations();
    TestDictOperations();
    TestJsonParsing();
    TestJsonPrinting();
    TestJsonSerialization();
    TestErrorHandling();
}

} // namespace test_json 