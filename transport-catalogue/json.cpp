#include "json.h"
#include <cctype>
#include <sstream>
#include <variant>
#include <iostream>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

void SkipWhitespace(istream& input) {
    while (input.peek() != EOF && isspace(input.peek())) {
        input.get();
    }
}

Node LoadArray(istream& input) {
    Array result;
    
    SkipWhitespace(input);
    if (input.peek() == ']') {
        input.get();
        return Node(std::move(result));
    }
    
    while (true) {
        SkipWhitespace(input);
        result.push_back(LoadNode(input));
        SkipWhitespace(input);
        
        char c = input.get();
        if (c == ']') {
            break;
        } else if (c != ',') {
            throw ParsingError("Expected ',' or ']' in array");
        }
    }

    return Node(std::move(result));
}

Node LoadNumber(istream& input) {
    string num_str;
    bool is_double = false;
    
    // Handle negative numbers
    if (input.peek() == '-') {
        num_str += input.get();
    }
    
    // Read digits before decimal point
    while (input.peek() != EOF && isdigit(input.peek())) {
        num_str += input.get();
    }
    
    // Check for decimal point
    if (input.peek() == '.') {
        is_double = true;
        num_str += input.get();
        
        // Read digits after decimal point
        while (input.peek() != EOF && isdigit(input.peek())) {
            num_str += input.get();
        }
    }
    
    // Check for exponent
    if (input.peek() == 'e' || input.peek() == 'E') {
        is_double = true;
        num_str += input.get();
        
        // Handle sign in exponent
        if (input.peek() == '+' || input.peek() == '-') {
            num_str += input.get();
        }
        
        // Read exponent digits
        while (input.peek() != EOF && isdigit(input.peek())) {
            num_str += input.get();
        }
    }
    
    if (num_str.empty()) {
        throw ParsingError("Invalid number");
    }
    
    if (is_double) {
        double value = stod(num_str);
        return Node(value);
    } else {
        int value = stoi(num_str);
        return Node(value);
    }
}

Node LoadString(istream& input) {
    string result;
    
    while (true) {
        char c = input.get();
        if (c == '"') {
            break;
        } else if (c == '\\') {
            char escape = input.get();
            switch (escape) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case 'r': result += '\r'; break;
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                default: throw ParsingError("Invalid escape sequence");
            }
        } else if (c == EOF) {
            throw ParsingError("Unterminated string");
        } else {
            result += c;
        }
    }
    
    return Node(std::move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    
    SkipWhitespace(input);
    if (input.peek() == '}') {
        input.get();
        return Node(std::move(result));
    }
    
    while (true) {
        SkipWhitespace(input);
        
        // Expect opening quote
        if (input.get() != '"') {
            throw ParsingError("Expected '\"' at start of key");
        }
        
        string key = LoadString(input).AsString();
        
        SkipWhitespace(input);
        
        // Expect colon
        if (input.get() != ':') {
            throw ParsingError("Expected ':' after key");
        }
        
        SkipWhitespace(input);
        result.insert({std::move(key), LoadNode(input)});
        
        SkipWhitespace(input);
        char c = input.get();
        if (c == '}') {
            break;
        } else if (c != ',') {
            throw ParsingError("Expected ',' or '}' in object");
        }
    }

    return Node(std::move(result));
}

Node LoadBool(istream& input) {
    char first = input.get();
    if (first == 't') {
        if (input.get() != 'r' || input.get() != 'u' || input.get() != 'e') {
            throw ParsingError("Invalid boolean value");
        }
        SkipWhitespace(input);
        char next = input.peek();
        if (next != EOF && next != ',' && next != ']' && next != '}' && !isspace(next)) {
            throw ParsingError("Invalid boolean value: extra characters after true");
        }
        return Node(true);
    } else if (first == 'f') {
        if (input.get() != 'a' || input.get() != 'l' || input.get() != 's' || input.get() != 'e') {
            throw ParsingError("Invalid boolean value");
        }
        SkipWhitespace(input);
        char next = input.peek();
        if (next != EOF && next != ',' && next != ']' && next != '}' && !isspace(next)) {
            throw ParsingError("Invalid boolean value: extra characters after false");
        }
        return Node(false);
    }
    throw ParsingError("Invalid boolean value");
}

Node LoadNull(istream& input) {
    if (input.get() != 'n' || input.get() != 'u' || input.get() != 'l' || input.get() != 'l') {
        throw ParsingError("Invalid null value");
    }
    SkipWhitespace(input);
    char next = input.peek();
    if (next != EOF && next != ',' && next != ']' && next != '}' && !isspace(next)) {
        throw ParsingError("Invalid null value: extra characters after null");
    }
    return Node(nullptr);
}

Node LoadNode(istream& input) {
    SkipWhitespace(input);
    
    char c = input.peek();
    
    if (c == '[') {
        input.get();
        return LoadArray(input);
    } else if (c == '{') {
        input.get();
        return LoadDict(input);
    } else if (c == '"') {
        input.get();
        return LoadString(input);
    } else if (c == 't' || c == 'f') {
        return LoadBool(input);
    } else if (c == 'n') {
        return LoadNull(input);
    } else if (c == '-' || isdigit(c)) {
        return LoadNumber(input);
    } else {
        throw ParsingError("Unexpected character: " + string(1, c));
    }
}

}  // namespace


const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("Node is not an array");
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsDict()) {
        throw logic_error("Node is not a map");
    }
    return std::get<Dict>(value_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("Node is not an int");
    }
    return std::get<int>(value_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw logic_error("Node is not a double");
    }
    if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }
    return std::get<double>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("Node is not a bool");
    }
    return std::get<bool>(value_);
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("Node is not a string");
    }
    return std::get<string>(value_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsDict() const {
    return std::holds_alternative<Dict>(value_);
}

Document::Document(Node root) : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    Node node = doc.GetRoot();
    node.PrintNode(node, output);
}

std::string GetStringValue(const Dict& dict, const std::string& field_name) {
    auto it = dict.find(field_name);
    if (it == dict.end()) {
        throw ParsingError("Field '" + field_name + "' not found");
    }
    if (!it->second.IsString()) {
        throw ParsingError("Field '" + field_name + "' is not a string");
    }
    return it->second.AsString();
}

int GetIntValue(const Dict& dict, const std::string& field_name) {
    auto it = dict.find(field_name);
    if (it == dict.end()) {
        throw ParsingError("Field '" + field_name + "' not found");
    }
    if (!it->second.IsInt()) {
        throw ParsingError("Field '" + field_name + "' is not an integer");
    }
    return it->second.AsInt();
}

double GetDoubleValue(const Dict& dict, const std::string& field_name) {
    auto it = dict.find(field_name);
    if (it == dict.end()) {
        throw ParsingError("Field '" + field_name + "' not found");
    }
    if (!it->second.IsDouble()) {
        throw ParsingError("Field '" + field_name + "' is not a number");
    }
    return it->second.AsDouble();
}

Node CreateErrorResponse(int request_id, const std::string& error_message) {
    Dict response;
    response["request_id"] = Node(request_id);
    response["error_message"] = Node(error_message);
    return Node(response);
}

Node CreateSuccessResponse(int request_id, const Dict& data) {
    Dict response = data;
    response["request_id"] = Node(request_id);
    return Node(response);
}

}  // namespace json 