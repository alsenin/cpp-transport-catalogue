#include "json_builder.h"
#include <utility>
#include <stdexcept>

namespace json
{

    // Реализация методов DictContext
    Builder::KeyContext Builder::DictContext::Key(std::string key)
    {
        builder_.Key(std::move(key));
        return KeyContext(builder_);
    }

    Builder &Builder::DictContext::EndDict()
    {
        return builder_.EndDict();
    }

    // Реализация методов KeyContext
    Builder::ValueContext Builder::KeyContext::Value(json::Node value)
    {
        builder_.Value(std::move(value));
        return ValueContext(builder_);
    }

    Builder::ValueContext Builder::KeyContext::Value(std::string value)
    {
        return Value(json::Node(std::move(value)));
    }

    Builder::ValueContext Builder::KeyContext::Value(int value)
    {
        return Value(json::Node(value));
    }

    Builder::ValueContext Builder::KeyContext::Value(double value)
    {
        return Value(json::Node(value));
    }

    Builder::ValueContext Builder::KeyContext::Value(bool value)
    {
        return Value(json::Node(value));
    }

    Builder::DictContext Builder::KeyContext::StartDict()
    {
        return builder_.StartDict();
    }

    Builder::ArrayContext Builder::KeyContext::StartArray()
    {
        return builder_.StartArray();
    }

    // Реализация методов ValueContext
    Builder::KeyContext Builder::ValueContext::Key(std::string key)
    {
        builder_.Key(std::move(key));
        return KeyContext(builder_);
    }

    Builder &Builder::ValueContext::EndDict()
    {
        return builder_.EndDict();
    }

    // Реализация методов ArrayContext
    Builder::ArrayContext &Builder::ArrayContext::Value(json::Node value)
    {
        builder_.Value(std::move(value));
        return *this; // Возвращаем тот же контекст для fluent API
    }

    Builder::ArrayContext &Builder::ArrayContext::Value(std::string value)
    {
        return Value(json::Node(std::move(value)));
    }

    Builder::ArrayContext &Builder::ArrayContext::Value(int value)
    {
        return Value(json::Node(value));
    }

    Builder::ArrayContext &Builder::ArrayContext::Value(double value)
    {
        return Value(json::Node(value));
    }

    Builder::ArrayContext &Builder::ArrayContext::Value(bool value)
    {
        return Value(json::Node(value));
    }

    Builder::DictContext Builder::ArrayContext::StartDict()
    {
        return builder_.StartDict();
    }

    Builder::ArrayContext Builder::ArrayContext::StartArray()
    {
        return builder_.StartArray();
    }

    Builder &Builder::ArrayContext::EndArray()
    {
        return builder_.EndArray();
    }

    // Реализация методов Builder
    Builder::Builder()
        : root_(nullptr), nodes_stack_(), current_key_(), state_(BuilderState::EMPTY) {}

    Builder::~Builder() = default;

    void Builder::CheckState(BuilderState expected, const std::string &operation)
    {
        if (state_ != expected)
        {
            throw std::logic_error(operation + " called in wrong state");
        }
    }

    void Builder::TransitionTo(BuilderState new_state)
    {
        state_ = new_state;
    }

    void Builder::AddNode(json::Node node)
    {
        if (nodes_stack_.empty())
        {
            root_ = std::move(node);
            TransitionTo(BuilderState::READY_TO_BUILD);
            return;
        }

        auto &top = nodes_stack_.back();
        if (state_ == BuilderState::DICT_EXPECTING_VALUE)
        {
            std::string key = std::move(current_key_);
            top->AsDict().emplace(key, std::move(node));
            current_key_.clear();
            TransitionTo(BuilderState::DICT_EXPECTING_KEY);
        }
        else if (state_ == BuilderState::ARRAY_EXPECTING_VALUE)
        {
            top->AsArray().push_back(std::move(node));
        }
        else
        {
            throw std::logic_error("AddNode called in wrong state");
        }
    }

    // Методы для начала построения - возвращают контексты
    Builder::DictContext Builder::StartDict()
    {
        if (state_ != BuilderState::DICT_EXPECTING_VALUE &&
            state_ != BuilderState::ARRAY_EXPECTING_VALUE &&
            state_ != BuilderState::EMPTY)
        {
            throw std::logic_error("StartDict called in wrong state");
        }

        auto dict = json::Node(json::Dict());
        if (nodes_stack_.empty())
        {
            root_ = dict;
            nodes_stack_.push_back(&root_);
        }
        else
        {
            auto &top = nodes_stack_.back();
            if (state_ == BuilderState::DICT_EXPECTING_VALUE)
            {
                std::string key = std::move(current_key_);
                top->AsDict().emplace(key, dict);
                nodes_stack_.push_back(&top->AsDict().at(key));
            }
            else
            {
                top->AsArray().push_back(dict);
                nodes_stack_.push_back(&top->AsArray().back());
            }
            current_key_.clear();
        }
        TransitionTo(BuilderState::DICT_EXPECTING_KEY);
        return DictContext(*this);
    }

    Builder::ArrayContext Builder::StartArray()
    {
        if (state_ != BuilderState::DICT_EXPECTING_VALUE &&
            state_ != BuilderState::ARRAY_EXPECTING_VALUE &&
            state_ != BuilderState::EMPTY)
        {
            throw std::logic_error("StartArray called in wrong state");
        }

        auto array = json::Node(json::Array());
        if (nodes_stack_.empty())
        {
            root_ = array;
            nodes_stack_.push_back(&root_);
        }
        else
        {
            auto &top = nodes_stack_.back();
            if (state_ == BuilderState::DICT_EXPECTING_VALUE)
            {
                std::string key = std::move(current_key_);
                top->AsDict().emplace(key, array);
                nodes_stack_.push_back(&top->AsDict().at(key));
            }
            else
            {
                top->AsArray().push_back(array);
                nodes_stack_.push_back(&top->AsArray().back());
            }
            current_key_.clear();
        }
        TransitionTo(BuilderState::ARRAY_EXPECTING_VALUE);
        return ArrayContext(*this);
    }

    // Методы для простых значений
    Builder &Builder::Value(json::Node value)
    {
        if (state_ == BuilderState::EMPTY)
        {
            root_ = std::move(value);
            TransitionTo(BuilderState::READY_TO_BUILD);
            return *this;
        }

        if (state_ != BuilderState::DICT_EXPECTING_VALUE &&
            state_ != BuilderState::ARRAY_EXPECTING_VALUE)
        {
            throw std::logic_error("Value called in wrong state");
        }

        AddNode(std::move(value));
        return *this;
    }

    Builder &Builder::Value(std::string value)
    {
        return Value(json::Node(std::move(value)));
    }

    Builder &Builder::Value(int value)
    {
        return Value(json::Node(value));
    }

    Builder &Builder::Value(double value)
    {
        return Value(json::Node(value));
    }

    Builder &Builder::Value(bool value)
    {
        return Value(json::Node(value));
    }

    // Внутренние методы для контекстов
    Builder &Builder::Key(std::string key)
    {
        CheckState(BuilderState::DICT_EXPECTING_KEY, "Key");
        if (nodes_stack_.empty())
        {
            throw std::logic_error("Key called without StartDict");
        }
        current_key_ = std::move(key);
        TransitionTo(BuilderState::DICT_EXPECTING_VALUE);
        return *this;
    }

    Builder &Builder::EndDict()
    {
        CheckState(BuilderState::DICT_EXPECTING_KEY, "EndDict");
        if (nodes_stack_.empty())
        {
            throw std::logic_error("EndDict called without StartDict");
        }
        nodes_stack_.pop_back();
        if (nodes_stack_.empty())
        {
            TransitionTo(BuilderState::READY_TO_BUILD);
        }
        else
        {
            auto &parent = nodes_stack_.back();
            if (parent->IsDict())
            {
                TransitionTo(BuilderState::DICT_EXPECTING_KEY);
            }
            else
            {
                TransitionTo(BuilderState::ARRAY_EXPECTING_VALUE);
            }
        }
        return *this;
    }

    Builder &Builder::EndArray()
    {
        CheckState(BuilderState::ARRAY_EXPECTING_VALUE, "EndArray");
        if (nodes_stack_.empty())
        {
            throw std::logic_error("EndArray called without StartArray");
        }
        nodes_stack_.pop_back();
        if (nodes_stack_.empty())
        {
            TransitionTo(BuilderState::READY_TO_BUILD);
        }
        else
        {
            auto &parent = nodes_stack_.back();
            if (parent->IsDict())
            {
                TransitionTo(BuilderState::DICT_EXPECTING_KEY);
            }
            else
            {
                TransitionTo(BuilderState::ARRAY_EXPECTING_VALUE);
            }
        }
        return *this;
    }

    json::Node Builder::Build()
    {
        CheckState(BuilderState::READY_TO_BUILD, "Build");
        if (nodes_stack_.empty() && root_.IsNull())
        {
            throw std::logic_error("Build called on empty object");
        }
        return std::move(root_);
    }

} // namespace json
