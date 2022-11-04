#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

#include "json.h"

namespace json {

class Builder {
    class BaseContext;

    class DictValueContext;

    class DictItemContext;

    class ArrayItemContext;

public:
    DictValueContext Key(std::string key);

    BaseContext Value(json::Value value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();

    BaseContext EndDict();

    BaseContext EndArray();

    json::Node Build();

private:
    json::Node root_;
    std::vector<json::Node*> nodes_stack_;
    bool is_key_ = false;

    class BaseContext {
    public:
        explicit BaseContext(Builder& builder) : builder_(builder) {
        }

        DictValueContext Key(std::string key) {
            return builder_.Key(std::move(key));
        }

        BaseContext Value(json::Value value) {
            return builder_.Value(std::move(value));
        }

        DictItemContext StartDict() {
            return builder_.StartDict();
        }

        ArrayItemContext StartArray() {
            return builder_.StartArray();
        }

        BaseContext EndDict() {
            return builder_.EndDict();
        }

        BaseContext EndArray() {
            return builder_.EndArray();
        }

        json::Node Build() {
            return builder_.Build();
        }

        Builder& GetBuilder() const {
            return builder_;
        }

    private:
        Builder& builder_;
    };

    class DictItemContext final : public BaseContext {
    public:
        DictItemContext(BaseContext context) : BaseContext(context.GetBuilder()) {
        }

        BaseContext Value(json::Value) = delete;

        DictItemContext StartDict() = delete;

        ArrayItemContext StartArray() = delete;

        BaseContext EndArray() = delete;

        json::Node Build() = delete;
    };

    class DictValueContext final : public BaseContext {
    public:
        DictValueContext(BaseContext context) : BaseContext(context.GetBuilder()) {
        }

        DictValueContext Key(std::string) = delete;

        BaseContext EndDict() = delete;

        BaseContext EndArray() = delete;

        json::Node Build() = delete;

        DictItemContext Value(json::Value value) {
            return BaseContext::Value(std::move(value));
        }
    };

    class ArrayItemContext final : public BaseContext {
    public:
        ArrayItemContext(BaseContext context) : BaseContext(context.GetBuilder()) {
        }

        DictValueContext Key(std::string) = delete;

        BaseContext EndDict() = delete;

        json::Node Build() = delete;

        ArrayItemContext Value(json::Value value) {
            return BaseContext::Value(std::move(value));
        }
    };

    bool IsSingleValueRoot();

    bool NotAKeyValue();

    template<typename T>
    void StartContainer();

};

template<typename T>
void Builder::StartContainer() {
    if (root_.IsNull() && nodes_stack_.empty()) {
        json::Node* cont_ptr = reinterpret_cast<json::Node*> (&root_.GetValue().emplace<T>());
        nodes_stack_.push_back(cont_ptr);
    } else if ((!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) && is_key_) {
        is_key_ = false;
        nodes_stack_.back()->GetValue() = T{};
    } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        json::Node* cont_ptr = static_cast<json::Node*> (&std::get<json::Array>(
                nodes_stack_.back()->GetValue()).emplace_back(T{}));
        nodes_stack_.push_back(cont_ptr);
    }
}

} // namespace json
