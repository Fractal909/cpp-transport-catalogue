#include "json_builder.h"

#include <variant>
#include <iostream>

namespace json {

    Builder::Builder()
        :last_com_(), init_(), last_keys_(), node_(), nodes_stack_()
    {}

    Builder::KeyeItemContext Builder::Key(std::string str) {

        if (last_com_ == Command::KEY) {
            throw std::logic_error("Key after key");
        }
        if (last_com_ == Command::SA) {
            throw std::logic_error("Key after SA");
        }

        last_keys_.push_back(std::move(str));

        last_com_ = Command::KEY;
        return BaseContext{ *this };
    }

    Builder::BaseContext Builder::Value(Node::Value val) {

        if (last_com_ == Command::SD) {
            throw std::logic_error("Val after SD");
        }
        if (last_com_ == Command::VALUE && nodes_stack_.empty()) {
            throw std::logic_error("Val after Val");
        }
        if (last_com_ == Command::VALUE && !(std::holds_alternative<Array>(nodes_stack_.back()->GetValue()))) {
            throw std::logic_error("Val after Val");
        }


        InsertValToNode(std::move(val));

        last_com_ = Command::VALUE;
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (last_com_ == Command::SD) {
            throw std::logic_error("SD after SD");
        }
        if (last_com_ == Command::VALUE && nodes_stack_.empty()) {
            throw std::logic_error("SD after Val");
        }
        if (last_com_ == Command::VALUE && !(std::holds_alternative<Array>(nodes_stack_.back()->GetValue()))) {
            throw std::logic_error("SD after Val");
        }


        Node* ptr_to_stack = InsertValToNode(std::move(Dict{}));
        nodes_stack_.push_back(ptr_to_stack);


        last_com_ = Command::SD;
        return BaseContext{ *this };
    }

    Builder::BaseContext Builder::EndDict() {

        if (nodes_stack_.empty()) {
            throw std::logic_error("Trying ED before SD or SA");
        }

        Node* node_ptr;
        if (nodes_stack_.empty()) {
            node_ptr = &node_;
        }
        else {
            node_ptr = nodes_stack_.back();
        }
        if (std::holds_alternative<Array>(node_ptr->GetValue())) {
            throw std::logic_error("Trying EndDict after StartArray");
        }


        if (last_com_ == Command::KEY) {
            throw std::logic_error("ED after key");
        }
        if (last_com_ == Command::SA) {
            throw std::logic_error("ED after SA");
        }

        nodes_stack_.pop_back();
        last_com_ = Command::ED;
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (last_com_ == Command::SD) {
            throw std::logic_error("SA after SD");
        }
        if (last_com_ == Command::VALUE && nodes_stack_.empty()) {
            throw std::logic_error("SA after Val");
        }
        if (last_com_ == Command::VALUE && !(std::holds_alternative<Array>(nodes_stack_.back()->GetValue()))) {
            throw std::logic_error("SA after Val");
        }


        Node* ptr_to_stack = InsertValToNode(std::move(Array{}));
        nodes_stack_.push_back(ptr_to_stack);


        last_com_ = Command::SA;
        return BaseContext{ *this };
    }

    Builder::BaseContext Builder::EndArray() {

        if (nodes_stack_.empty()) {
            throw std::logic_error("Trying EA before SA or SD");
        }

        Node* node_ptr;
        if (nodes_stack_.empty()) {
            node_ptr = &node_;
        }
        else {
            node_ptr = nodes_stack_.back();
        }
        if (std::holds_alternative<Dict>(node_ptr->GetValue())) {
            throw std::logic_error("Trying EndArray after StartDict");
        }


        if (last_com_ == Command::KEY) {
            throw std::logic_error("EA after key");
        }
        if (last_com_ == Command::SD) {
            throw std::logic_error("EA after SD");
        }


        nodes_stack_.pop_back();
        last_com_ = Command::EA;
        return *this;
    }

    Node Builder::Build() {

        if (!init_) {
            throw std::logic_error("Build not init");
        }
        if (last_com_ == Command::KEY) {
            throw std::logic_error("Build after key");
        }
        if (last_com_ == Command::SD) {
            throw std::logic_error("Build after SD");
        }
        if (last_com_ == Command::SA) {
            throw std::logic_error("Build after SA");
        }
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Not complete array or dict");
        }
        return node_;

    }

    Node* Builder::InsertValToNode(Node::Value val) {

        Node* node_ptr;
        if (nodes_stack_.empty()) {
            node_ptr = &node_;
            init_ = true;
        }
        else {
            node_ptr = nodes_stack_.back();
        }

        Node n;
        n.GetValue() = std::move(val);

        if (std::holds_alternative<Dict>(node_ptr->GetValue())) {

            Dict& d = std::get<Dict>(node_ptr->GetValue());
            auto iter = d.insert({ std::move(last_keys_.back()), std::move(n) });
            auto ptr = &(iter.first->second);
            last_keys_.pop_back();
            return ptr;
        }
        else if (std::holds_alternative<Array>(node_ptr->GetValue())) {

            Array& a = std::get<Array>(node_ptr->GetValue());
            auto& iter = a.emplace_back(std::move(n));
            auto ptr = &iter;
            return ptr;
        }
        else {
            *node_ptr = n;
            return node_ptr;
        }
    }
}

