// Вставьте сюда решение из предыдущего спринта
#pragma once

#include "json.h"
#include <optional>
#include <vector>
#include <string>


namespace json
{
	
	

	class Builder
	{
	public:
		class ItemContext;
		class KeyItemContext;
		class ArrayItemContext;

		Builder();
		ItemContext StartDict();
		ArrayItemContext StartArray();
		KeyItemContext Key(std::string);
		Builder& Value(Node::Value);
		Builder& EndDict();
		Builder& EndArray();
		Node Build();

		

	private:
		std::optional<std::string> keys_;
		Node root_ = nullptr;
		std::vector<Node*> nodes_stack_;
		Node GetNode(json::Node::Value value);

		template <typename T>
		void  Helper(T t) {
			auto top_node = nodes_stack_.back();

			if (top_node->IsDict() && keys_.has_value())
			{
				auto& dict = std::get<Dict>(top_node->GetValue());
				auto it = dict.emplace(keys_.value(), t);
				nodes_stack_.push_back(&it.first->second);
				keys_.reset();
			}
			else if (top_node->IsArray())
			{
				auto& arr = std::get<Array>(top_node->GetValue());
				arr.emplace_back(t);
				*(nodes_stack_.back()) = arr;
				nodes_stack_.push_back(&arr.back());
			}
			else if (top_node->IsNull())
			{
				top_node->GetValue() = t;
			}
			else
				throw std::logic_error("Wrong context of StartArray()");

		}

	};

	class Builder::ItemContext
	{
	public:
		ItemContext(Builder& builder) : main_(builder) {}

		KeyItemContext Key(std::string);
		Builder& EndDict();

	private:
		Builder& main_;
	};

	class Builder::KeyItemContext
	{
	public:
		KeyItemContext(Builder& builder) : main_(builder) {}

		ItemContext Value(Node::Value);

		ArrayItemContext StartArray();
		ItemContext StartDict();

	private:
		Builder& main_;
	};


	class Builder::ArrayItemContext
	{
	public:
		ArrayItemContext(Builder& builder) : main_(builder) {}

		ArrayItemContext StartArray();
		ItemContext StartDict();
		ArrayItemContext Value(Node::Value);
		Builder& EndArray();

	private:
		Builder& main_;
	};
} //namespace json 