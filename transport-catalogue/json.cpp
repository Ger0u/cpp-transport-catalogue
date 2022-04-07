#include "json.h"

#include <cmath>
using namespace std;

namespace json {

Node LoadArray(istream& input) {
    Array result;
    
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (c != ']') {
        throw ParsingError("missing character ']'"s);
    }
    
    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(istream& input) {
    string line;
    char c = input.get();
    for (bool escape = false;
         c != '\"' || escape;
         c = input.get())
    {
        if (c == char_traits<char>::eof()) {
            throw ParsingError("missing character '\"'"s);
        }
        if (escape) {
            if (c == 'n') {
                line += '\n';
            } else if (c == 'r') {
                line += '\r';
            } else if (c == '\"') {
                line += '\"';
            } else if (c == 't') {
                line += '\t';
            } else if (c == '\\') {
                line += '\\';
            } else {
                throw ParsingError("unknown escape sequence \'\\"s + c + '\'');
            }
            escape = false;
        } else {
            if (c == '\\') {
                escape = true;
            } else {
                line += c;
            }
        }
    }
    return Node(move(line));
}

Node LoadDict(istream& input) {
    Dict result;

    char c;
    bool flag = false;
    for (; input >> c && c != '}';) {
        if (flag) {
            if (c == ',') {
                input >> c;
            } else {
                throw ParsingError("missing character ','"s);
            }
        } else {
            flag = true;
        }
        
        if (c != '\"') {
            throw ParsingError("missing character '\"'"s);
        }

        string key = LoadString(input).AsString();
        input >> c;
        if (c != ':') {
            throw ParsingError("missing character ':'"s);
        }
        result.insert({move(key), LoadNode(input)});
    }
    
    if (c != '}') {
        throw ParsingError("missing character '}'"s);
    }

    return Node(move(result));
}
    
Node LoadOther(istream& input) {
    if (input.peek() == 't') {
        for (char c : "true"sv) {
            if (c != input.get()) {
                throw ParsingError("unknown data type is used"s);
            }
        }
        return true;
    }
    if (input.peek() == 'f') {
        for (char c : "false"sv) {
            if (c != input.get()) {
                throw ParsingError("unknown data type is used"s);
            }
        }
        return false;
    }
    for (char c : "null"sv) {
        if (c != input.get()) {
            throw ParsingError("unknown data type is used"s);
        }
    }
    return {};
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (isdigit(c) || c == '-' || c == '+') {
        input.putback(c);
        return LoadNumber(input);
    } else {
        input.putback(c);
        return LoadOther(input);
    }
}
    
Node::Node(nullptr_t content)
: content_(content) {
}
    
Node::Node(int content)
: content_(content) {
}
    
Node::Node(double content)
: content_(content) {
}
    
Node::Node(string content)
: content_(move(content)) {
}
    
Node::Node(bool content)
: content_(content) {
}
    
Node::Node(Array content)
: content_(move(content)) {
}
    
Node::Node(Dict content)
: content_(move(content)) {
}
    
bool Node::operator==(const Node& other) const {
    return content_ == other.content_;
}
    
bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("not int"s);
    }
    return get<int>(content_);
}
    
double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("not double"s);
    }
    return IsInt()
         ? get<int>(content_)
         : get<double>(content_);
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("not string"s);
    }
    return get<string>(content_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("not bool"s);
    }
    return get<bool>(content_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("not Array"s);
    }
    return get<Array>(content_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("not Dict"s);
    }
    return get<Dict>(content_);
}
    
bool Node::IsNull() const {
    return holds_alternative<std::nullptr_t>(content_);
}

bool Node::IsInt() const {
    return holds_alternative<int>(content_);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(content_);
}

bool Node::IsString() const {
    return holds_alternative<string>(content_);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(content_);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(content_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(content_);
}

Document::Document(Node root)
    : root_(move(root)) {
}
    
bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}
    
bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, ostream& output) {
    Print(doc.GetRoot(), output);
}
    
void Tabulation(ostream& output, size_t tabulation) {
    for (size_t i = 0; i < tabulation; ++i) {
        output << "    "sv;
    }
}
    
void Print(const Node& node, ostream& output, size_t tabulation) {
    if (node.IsArray()) {
        output << "["sv;
        for (auto under_node = node.AsArray().begin();
             under_node != node.AsArray().end();
             ++under_node)
        {
            if (under_node != node.AsArray().begin()) {
                output << ","sv;
            }
            output << endl;
            Tabulation(output, tabulation + 1);
            Print(*under_node, output, tabulation + 1);
        }
        output << endl;
        Tabulation(output, tabulation);
        output << "]"sv;
    } else if (node.IsMap()) {
        output << "{"sv;
        for (auto under_node = node.AsMap().begin();
             under_node != node.AsMap().end();
             ++under_node)
        {
            if (under_node != node.AsMap().begin()) {
                output << ","sv;
            }
            output << endl;
            Tabulation(output, tabulation + 1);
            Print(under_node->first, output, tabulation + 1);
            output << ": "sv;
            Print(under_node->second, output, tabulation + 1);
        }
        output << endl;
        Tabulation(output, tabulation);
        output << "}"sv;
    } else if (node.IsString()) {
        output << "\""sv;
        StringWithEscape(node.AsString(), output);
        output << "\""sv;
    } else if (node.IsPureDouble()) {
        output << node.AsDouble();
    } else if (node.IsInt()) {
        output << node.AsInt();
    } else if (node.IsNull()) {
        output << "null"sv;
    } else if (node.IsBool()) {
        output << (node.AsBool() ? "true"sv : "false"sv);
    }
}
   
void StringWithEscape(string_view str, ostream& output) {
    for (const char& c : str) {
        output <<  (c == '\n' ? "\\n"sv
                  : c == '\r' ? "\\r"sv
                  : c == '\"' ? "\\\""sv
                  : c == '\t' ? "\\t"sv
                  : c == '\\' ? "\\\\"sv
                  : string_view(&c, 1));
    }
}
    
}  // namespace json