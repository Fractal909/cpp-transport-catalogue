#include "json.h"

#include <regex>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

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
                throw ParsingError("Array parsing error");
            }

            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
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
            }
            else {
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
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;

            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c != '}') {
                throw ParsingError("Dict parsing error");
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (std::isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNumber(input);
            }
            else if (c == 't') {
                std::string str;
                str.resize(3);
                input.read(str.data(), 3);
                if (str == "rue") {
                    char c;
                    if (input.get(c)) {
                        if ((c == ' ' || c == ',' || c == '}' || c == '\n')) {
                            input.putback(c);
                            return Node{ true };
                        }
                        else {
                            throw ParsingError("PE");
                        }
                    }
                    return Node{ true };
                }
                else {
                    throw ParsingError("PE");
                }
            }
            else if (c == 'f') {
                std::string str;
                str.resize(4);
                input.read(str.data(), 4);
                if (str == "alse") {
                    char c;
                    if (input.get(c)) {
                        if (c == ' ' || c == ',' || c == '}' || c == '\n') {
                            input.putback(c);
                            return Node{ false };
                        }
                        else {
                            throw ParsingError("PE");
                        }
                    }
                    return Node{ false };
                }
                else {
                    throw ParsingError("PE");
                }
            }
            else if (c == 'n') {
                std::string str;
                str.resize(3);
                input.read(str.data(), 3);
                if (str == "ull") {
                    char c;
                    if (input.get(c)) {
                        if ((c == ' ' || c == ',' || c == '}' || c == '\n')) {
                            input.putback(c);
                            return Node{ nullptr };
                        }
                        else {
                            throw ParsingError("PE");
                        }
                    }
                    return Node{ nullptr };
                }
                else {
                    throw ParsingError("PE");
                }
            }
            else {
                throw ParsingError("PE");
            }
        }

    }  // namespace

    Node::Node()
        :content_(nullptr_t(nullptr))
    {
    }
    Node::Node(nullptr_t nptr)
        :content_(nptr)
    {
    }
    Node::Node(Array array)
        :content_(array)
    {
    }
    Node::Node(Dict dict)
        :content_(dict)
    {
    }
    Node::Node(int val)
        :content_(val)
    {
    }
    Node::Node(double val)
        :content_(val)
    {
    }
    Node::Node(std::string str)
        :content_(str)
    {
    }
    Node::Node(bool b)
        :content_(b)
    {
    }



    bool Node::IsInt() const {
        return std::holds_alternative<int>(content_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(content_) || std::holds_alternative<double>(content_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(content_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(content_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(content_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<nullptr_t>(content_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(content_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(content_);
    }




    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(content_);
        }
        else {
            throw std::logic_error("LE");
        }
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(content_);
        }
        else {
            throw std::logic_error("LE");
        }
    }

    double Node::AsDouble() const {
        if (IsDouble()) {
            if (std::holds_alternative<int>(content_)) {
                return static_cast<double>(std::get<int>(content_));
            }
            else {
                return std::get<double>(content_);
            }
        }
        else {
            throw std::logic_error("LE");
        }
    }

    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(content_);
        }
        else {
            throw std::logic_error("LE");
        }
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(content_);
        }
        else {
            throw std::logic_error("LE");
        }
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(content_);
        }
        else {
            throw std::logic_error("LE");
        }
    }

    const NodeContent& Node::GetContent() const {
        return content_;
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetContent() == rhs.GetContent();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }





    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }


    void PrintString(const string& str, ostream& output) {
        output << '"';
        for (const auto& ch : str) {
            switch (ch)
            {
            case('"'):
                output << "\\\"";
                break;
            case('\\'):
                output << "\\\\";
                break;
            case('\n'):
                output << "\\n";
                break;
            case('\r'):
                output << "\\r";
                break;
            case('\t'):
                output << "\\t";
                break;
            default:
                output << ch;
                break;
            }
        }
        output << '"';
    }

    void PrintNode(const Node& node, std::ostream& output) {
        if (node.IsNull()) {
            output << "null"s;
        }
        else if (node.IsInt()) {
            output << node.AsInt();
        }
        else if (node.IsDouble()) {
            output << node.AsDouble();
        }
        else if (node.IsString()) {
            PrintString(node.AsString(), output);
        }
        else if (node.IsBool()) {
            if (node.AsBool()) {
                output << "true"s;
            }
            else {
                output << "false"s;
            }
        }
        else if (node.IsArray()) {
            output << '[';
            for (const auto& elem : node.AsArray()) {
                PrintNode(elem, output);
                if (&elem != &(node.AsArray().back())) {
                    output << ',';
                }
            }
            output << ']';
        }
        else if (node.IsMap()) {
            output << '{';
            for (const auto& elem : node.AsMap()) {
                PrintString(elem.first, output);
                output << " : "s;
                PrintNode(elem.second, output);
                if (&elem != &(*(std::prev(node.AsMap().end())))) {
                    output << ", ";
                }
            }
            output << '}';
        }
    }

    void Print(const Document& doc, std::ostream& output) {
        (void)&doc;
        (void)&output;

        // Реализуйте функцию самостоятельно
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json