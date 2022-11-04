#include "json_builder.h"

namespace json {

using namespace std::literals;

Builder::DictValueContext Builder::Key(std::string key) {
    if (root_.IsNull() || (!nodes_stack_.empty() && !nodes_stack_.back()->IsDict())) {
        throw std::logic_error("Incorrect Dict key insertion"s);
    }
    if (IsSingleValueRoot()) {
        throw std::logic_error("Incorrect Dict key insertion"s);
    }
    is_key_ = true;
    json::Node* value_ptr_ = &std::get<json::Dict>(nodes_stack_.back()->GetValue())[std::move(key)];
    nodes_stack_.push_back(value_ptr_);
    return BaseContext(*this);
}

Builder::BaseContext Builder::Value(json::Value value) {
    if (IsSingleValueRoot()) {
        throw std::logic_error("Multivalued node must be Array or Dict. Cannot insert value"s);
    }
    if (NotAKeyValue()) {
        throw std::logic_error("Trying to assign value to Dict with no key associated"s);
    }
    if (root_.IsNull() && nodes_stack_.empty()) {
        root_.GetValue() = std::move(value);
    } else if (nodes_stack_.back()->IsNull() && is_key_) {
        is_key_ = false;
        nodes_stack_.back()->GetValue() = std::move(value);
        nodes_stack_.pop_back();
    } else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        std::get<json::Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(value));
    }
    return BaseContext(*this);
}

Builder::ArrayItemContext Builder::StartArray() {
    if (IsSingleValueRoot()) {
        throw std::logic_error("Multivalued node must be Array or Dict. Cannot start Array"s);
    }
    if (NotAKeyValue()) {
        throw std::logic_error("Trying to assign Array to Dict with no key associated"s);
    }
    StartContainer<json::Array>();
    return BaseContext(*this);
}

Builder::DictItemContext Builder::StartDict() {
    if (IsSingleValueRoot()) {
        throw std::logic_error("Multivalued node must be Array or Dict. Cannot start Dict"s);
    }
    if (NotAKeyValue()) {
        throw std::logic_error("Trying to assign Dict to Dict with no key associated"s);
    }
    StartContainer<json::Dict>();
    return BaseContext(*this);
}

Builder::BaseContext Builder::EndArray() {
    if (root_.IsNull() || (!nodes_stack_.empty() && !nodes_stack_.back()->IsArray())) {
        throw std::logic_error("Trying to close incomplete sequence or no Array start found"s);
    }
    if (IsSingleValueRoot()) {
        throw std::logic_error("Trying to close incomplete sequence or no Array start found"s);
    }
    nodes_stack_.pop_back();
    return BaseContext(*this);
}

Builder::BaseContext Builder::EndDict() {
    if (root_.IsNull() || (!nodes_stack_.empty() && !nodes_stack_.back()->IsDict())) {
        throw std::logic_error("Trying to close incomplete sequence or no Dict start found"s);
    }
    if (IsSingleValueRoot()) {
        throw std::logic_error("Trying to close incomplete sequence or no Dict start found"s);
    }
    nodes_stack_.pop_back();
    return BaseContext(*this);
}

json::Node Builder::Build() {
    if (root_.IsNull()) {
        throw std::logic_error("Cannot build empty root"s);
    }
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Cannot build incomplete sequence"s);
    }
    return root_;
}

bool Builder::IsSingleValueRoot() {
    return !root_.IsNull() && nodes_stack_.empty();
}

bool Builder::NotAKeyValue() {
    return !nodes_stack_.empty() && nodes_stack_.back()->IsDict();
}

} // namespace json