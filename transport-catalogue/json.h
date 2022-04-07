#pragma once

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

namespace json {
    
class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    Node() = default;
    Node(std::nullptr_t content);
    Node(int content);
    Node(double content);
    Node(std::string content);
    Node(bool content);
    Node(Array content);
    Node(Dict content);
    
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;
    
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsArray() const;
    bool IsMap() const;

private:
    std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> content_;
};

class Document {
public:
    explicit Document(Node root);
    
    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);
    
Node LoadNode(std::istream& input);
    
Node LoadArray(std::istream& input);
    
Node LoadNumber(std::istream& input);
    
Node LoadString(std::istream& input);
    
Node LoadDict(std::istream& input);
    
Node LoadOther(std::istream& input);

void Print(const Document& doc, std::ostream& output);
    
void Tabulation(std::ostream& output, size_t tabulation);
    
void Print(const Node& node, std::ostream& output, size_t tabulation = 0);
    
void StringWithEscape(std::string_view str, std::ostream& output);

}  // namespace json