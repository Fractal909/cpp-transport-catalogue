#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>


namespace json {

    class Node;
    // —охраните объ€влени€ Dict и Array без изменени€
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeContent = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        /* –еализуйте Node, использу€ std::variant */

        //explicit Node(nullptr_t nptr);
        Node();
        /*explicit*/ Node(nullptr_t null);
        /*explicit*/ Node(Array array);
        /*explicit*/ Node(Dict map);
        /*explicit*/ Node(int value);
        /*explicit*/ Node(double val);
        /*explicit*/ Node(std::string value);
        /*explicit*/ Node(bool b);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const NodeContent& GetContent() const;
    private:
        NodeContent content_;
    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);


    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json