#include "json_builder.h"

#include <stdexcept>
#include <cassert>
using namespace std;

namespace json {

//--------------------Builder::DictKeyContext--------------------
    
Builder::DictKeyContext::DictKeyContext(Builder* builder)
: builder_(builder) {
}
        
Builder::DictItemContext Builder::DictKeyContext::Value(Node::Value value) {
    builder_->Value(move(value));
    return {builder_};
}
        
Builder::DictItemContext Builder::DictKeyContext::StartDict() {
    return builder_->StartDict();
}

Builder::ArrayItemContext Builder::DictKeyContext::StartArray() {
    return builder_->StartArray();
}

//--------------------Builder::DictItemContext--------------------
    
Builder::DictItemContext::DictItemContext(Builder* builder)
: builder_(builder) {
}
        
Builder::DictKeyContext Builder::DictItemContext::Key(string key) {
    return builder_->Key(move(key));
}

Builder& Builder::DictItemContext::EndDict() {
    return builder_->EndDict();
}
    
//--------------------Builder::ArrayItemContext--------------------
    
Builder::ArrayItemContext::ArrayItemContext(Builder* builder)
: builder_(builder) {
}
        
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    builder_->Value(move(value));
    return {builder_};
}
    
Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return builder_->StartDict();
}
    
Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return builder_->StartArray();
}
    
Builder& Builder::ArrayItemContext::EndArray() {
    return builder_->EndArray();
}
    
//--------------------Builder--------------------
    
Builder::DictKeyContext Builder::Key(string key) {
    CheckRoot();
    if (!nodes_stack_.empty() && holds_alternative<Dict>(*nodes_stack_.back())) {
        nodes_stack_.push_back(new Node::Value(move(key)));
    } else {
        throw std::logic_error("Calling Key in the wrong context"s);
    }
    return {this};
}
    
Builder& Builder::Value(Node::Value value) {
    CheckRoot();
    if (nodes_stack_.empty() || holds_alternative<string>(*nodes_stack_.back())
        || holds_alternative<Array>(*nodes_stack_.back()))
    {
        Node node_value(LoadNode(value));
        if (nodes_stack_.empty()) {
            root_ = move(node_value);
        } else if (string* key = get_if<string>(nodes_stack_.back()))
        {
            Dict* dict = get_if<Dict>(nodes_stack_[nodes_stack_.size() - 2]);
            dict->emplace(move(*key), move(node_value));
            nodes_stack_.pop_back();
        } else if (Array* arr = get_if<Array>(nodes_stack_.back())) {
            arr->push_back(move(node_value));
        } else {
            assert(false);
        }
    } else {
        throw std::logic_error("Calling Value in the wrong context"s);
    }
    return *this;
}
    
Builder::DictItemContext Builder::StartDict() {
    CheckRoot();
    if (nodes_stack_.empty() || holds_alternative<string>(*nodes_stack_.back())
        || holds_alternative<Array>(*nodes_stack_.back()))
    {
        nodes_stack_.push_back(new Node::Value(Dict()));
    } else {
        throw std::logic_error("Calling StartDict in the wrong context"s);
    }
    return {this};
}

Builder::ArrayItemContext Builder::StartArray() {
    CheckRoot();
    if (nodes_stack_.empty() || holds_alternative<string>(*nodes_stack_.back())
        || holds_alternative<Array>(*nodes_stack_.back()))
    {
        nodes_stack_.push_back(new Node::Value(Array()));
    } else {
        throw std::logic_error("Calling StartArray in the wrong context"s);
    }
    return {this};
}

Builder& Builder::EndDict() {
    CheckRoot();
    if (!nodes_stack_.empty() && holds_alternative<Dict>(*nodes_stack_.back())) {
        Node::Value value = move(*nodes_stack_.back());
        nodes_stack_.pop_back();
        Value(move(value));
    } else {
        throw std::logic_error("Calling EndDict in the wrong context"s);
    }
    return *this;
}
    
Builder& Builder::EndArray() {
    CheckRoot();
    if (!nodes_stack_.empty() && holds_alternative<Array>(*nodes_stack_.back())) {
        Node::Value value = move(*nodes_stack_.back());
        nodes_stack_.pop_back();
        Value(move(value));
    } else {
        throw std::logic_error("Calling EndArray in the wrong context"s);
    }
    return *this;
}

Node Builder::Build() {
    if (!root_) {
        throw std::logic_error("The json object is not ready"s);
    }
    return move(root_.value());
}
    
void Builder::CheckRoot() const {
    if (root_) {
        throw std::logic_error("The json object is ready"s);
    }
}
    
Node Builder::LoadNode(Node::Value value) {
    if (nullptr_t* val = get_if<nullptr_t>(&value)) {
        return {*val};
    } else if (Array* val = get_if<Array>(&value)) {
        return {move(*val)};
    } else if (Dict* val = get_if<Dict>(&value)) {
        return {move(*val)};
    } else if (bool* val = get_if<bool>(&value)) {
        return {*val};
    } else if (int* val = get_if<int>(&value)) {
        return {*val};
    } else if (double* val = get_if<double>(&value)) {
        return {*val};
    } else if (string* val = get_if<string>(&value)) {
        return {move(*val)};
    }
    assert(false);
}
    
} // namespace json