// Вставьте сюда решение из предыдущего спринта
#include  <cstddef>
#include "json_builder.h"


json::Builder::Builder() {
	nodes_stack_.push_back(&root_);
}


json::Builder::ItemContext json::Builder::Builder::StartDict()
{
	Helper(Dict());
	return *this;
}

json::Builder& json::Builder::EndDict()
{
	auto top_node = nodes_stack_.back();

	if (top_node->IsDict() && !keys_.has_value())
		nodes_stack_.pop_back();
	else
		throw std::logic_error("Dictionary must exist");

	return *this;
}

json::Builder::ArrayItemContext json::Builder::Builder::StartArray()
{
	Helper(Array());
	return *this;
}

json::Builder& json::Builder::EndArray()
{
	auto top_node = nodes_stack_.back();

	if (top_node->IsArray())
		nodes_stack_.pop_back();
	else
		throw std::logic_error("Array must exist");

	return *this;
}

json::Builder::KeyItemContext json::Builder::Key(std::string key)
{
	auto top_node = nodes_stack_.back();

	if (top_node->IsDict() && !keys_.has_value())
		keys_ = std::move(key);
	else
		throw std::logic_error("Wrong map key: " + key);

	return *this;
}

json::Builder& json::Builder::Value(Node::Value value)
{
	auto top_node = nodes_stack_.back();

	if (top_node->IsArray())
	{
		auto arr = top_node->AsArray();
		arr.push_back(GetNode(value));
		*(nodes_stack_.back()) = arr;
	}
	else if (top_node->IsDict() && keys_.has_value())
	{
		auto dict = top_node->AsDict();
		dict[keys_.value()].GetValue() = value;
		*(nodes_stack_.back()) = dict;
		keys_.reset();
	}
	else if (top_node->IsNull())
	{
		root_.GetValue() = value;
	}
	else
		throw std::logic_error("Wrong context of command Value()");
	return *this;
}

json::Node json::Builder::Build()
{
	if (nodes_stack_.size() == 1)
	{
		if (nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsDict() || nodes_stack_.back()->IsNull())
			throw std::logic_error("You must close Array or Dict before Build()");
		else
			return root_;
	}

	return root_;
}

json::Builder::KeyItemContext json::Builder::ItemContext::Key(std::string key)
{
	return main_.Key(key);
}

json::Builder& json::Builder::ItemContext::EndDict()
{
	return main_.EndDict();
}

json::Builder::ItemContext json::Builder::KeyItemContext::Value(Node::Value val)
{
	return main_.Value(val);
}

json::Builder::ArrayItemContext json::Builder::KeyItemContext::StartArray()
{
	return main_.StartArray();
}

json::Builder::ItemContext json::Builder::KeyItemContext::StartDict()
{
	return main_.StartDict();
}



json::Builder::ArrayItemContext json::Builder::ArrayItemContext::StartArray()
{
	return main_.StartArray();
}

json::Builder::ItemContext json::Builder::ArrayItemContext::StartDict()
{
	return main_.StartDict();
}
json::Builder::ArrayItemContext json::Builder::ArrayItemContext::Value(Node::Value val)
{
	return main_.Value(val);
}
json::Builder& json::Builder::ArrayItemContext::EndArray()
{
	return main_.EndArray();
}

json::Node json::Builder::GetNode(json::Node::Value value) {
	if (std::holds_alternative<int>(value))
		return Node(std::get<int>(value));
	else if (std::holds_alternative<double>(value))
		return Node(std::get<double>(value));
	else if (std::holds_alternative<std::string>(value))
		return Node(std::get<std::string>(value));
	else if (std::holds_alternative<std::nullptr_t>(value))
		return Node(std::get<std::nullptr_t>(value));
	else if (std::holds_alternative<bool>(value))
		return Node(std::get<bool>(value));
	else if (std::holds_alternative<Dict>(value))
		return Node(std::get<Dict>(value));
	else if (std::holds_alternative<Array>(value))
		return Node(std::get<Array>(value));
	else
		throw std::logic_error("Bad value");
}