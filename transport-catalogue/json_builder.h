#pragma once

#include "json.h"
#include <string>
#include <vector>
#include <optional>

namespace json {

class Builder final {
public:
    class DictKeyContext;
    class DictItemContext;
    class ArrayItemContext;
    friend DictKeyContext;
    friend DictItemContext;
    friend ArrayItemContext;
    
    class DictKeyContext final {
    public:
        DictKeyContext(Builder* builder);
        
        DictItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    private:
        Builder* builder_;
    };
    
    class DictItemContext final {
    public:
        DictItemContext(Builder* builder);
        
        DictKeyContext Key(std::string key);
        Builder& EndDict();
    private:
        Builder* builder_;
    };
    
    class ArrayItemContext final {
    public:
        ArrayItemContext(Builder* builder);
        
        ArrayItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    private:
        Builder* builder_;
    };
    
    DictKeyContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
    
private:
    std::optional<Node> root_;
    std::vector<Node::Value*> nodes_stack_;
    
    void CheckRoot() const;
    static Node LoadNode(Node::Value value);
};
    
} // namespace json