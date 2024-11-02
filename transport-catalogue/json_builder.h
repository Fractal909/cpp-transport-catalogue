#pragma once

#include "json.h"

#include <vector>
#include <string>

namespace json {

    enum Command {
        KEY,
        VALUE,
        SD,
        ED,
        SA,
        EA
    };

    class KeyItemContext;
    class ValueItemContext;
    class DictItemContext;
    class ArrayItemContext;
    class ArrayValItemContext;


    class Builder {
    public:

        KeyItemContext Key(std::string str);
        Builder& Value(Node::Value val);

        DictItemContext StartDict();
        Builder& EndDict();

        ArrayItemContext StartArray();
        Builder& EndArray();

        Node Build();

    private:
        Node* InsertValToNode(Node::Value val);

        Command last_com_;
        bool init = false;
        std::vector<std::string> last_keys_;
        Node node_ = nullptr;
        std::vector<Node*> nodes_stack_;
    };

    class KeyItemContext {
    public:
        KeyItemContext(Builder& builder)
            :builder_(builder)
        {}

        ValueItemContext Value(Node::Value val);
        DictItemContext StartDict();
        ArrayItemContext StartArray();

    private:
        Builder& builder_;
    };

    class ValueItemContext {
    public:
        ValueItemContext(Builder& builder)
            :builder_(builder)
        {}

        KeyItemContext Key(std::string str);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

    class DictItemContext {
    public:
        DictItemContext(Builder& builder)
            :builder_(builder)
        {}

        KeyItemContext Key(std::string str);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

    class ArrayItemContext {
    public:
        ArrayItemContext(Builder& builder)
            :builder_(builder)
        {}

        ArrayValItemContext Value(Node::Value val);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        Builder& builder_;
    };

    class ArrayValItemContext {
    public:
        ArrayValItemContext(Builder& builder)
            :builder_(builder)
        {}

        ArrayValItemContext Value(Node::Value val);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        Builder& builder_;
    };

}