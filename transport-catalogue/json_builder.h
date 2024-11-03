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



    class Builder {
    private:
        class BaseContext;
        class KeyeItemContext;
        class DictItemContext;
        class ArrayItemContext;

    public:
        Builder();

        KeyeItemContext Key(std::string str);
        BaseContext Value(Node::Value val);

        DictItemContext StartDict();
        BaseContext EndDict();

        ArrayItemContext StartArray();
        BaseContext EndArray();

        Node Build();

    private:
        Node* InsertValToNode(Node::Value val);

        Command last_com_;
        bool init_ = false;
        std::vector<std::string> last_keys_;
        Node node_ = nullptr;
        std::vector<Node*> nodes_stack_;



        class BaseContext {
        public:
            BaseContext(Builder& builder) : builder_(builder) {}
            KeyeItemContext Key(std::string key) {
                return builder_.Key(std::move(key));
            }
            BaseContext Value(Node::Value value) {
                return builder_.Value(std::move(value));
            }
            DictItemContext StartDict() {
                return builder_.StartDict();
            }
            BaseContext EndDict() {
                return builder_.EndDict();
            }
            ArrayItemContext StartArray() {
                return builder_.StartArray();
            }
            BaseContext EndArray() {
                return builder_.EndArray();
            }
            Node Build() {
                return builder_.Build();
            }
        private:
            Builder& builder_;
        };

        class KeyeItemContext : public BaseContext {
        public:
            KeyeItemContext(BaseContext base) : BaseContext(base) {}

            DictItemContext Value(Node::Value value) {
                return BaseContext::Value(std::move(value));
            }
            Node Build() = delete;
            KeyeItemContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
        };

        class DictItemContext : public BaseContext {
        public:
            DictItemContext(BaseContext base) : BaseContext(base) {}

            Node Build() = delete;
            BaseContext Value(Node::Value value) = delete;
            BaseContext EndArray() = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(BaseContext base) : BaseContext(base) {}

            ArrayItemContext Value(Node::Value value) {
                return BaseContext::Value(std::move(value));
            }
            Node Build() = delete;
            KeyeItemContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
        };
    };



}