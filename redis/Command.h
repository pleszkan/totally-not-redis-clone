#pragma once
#include <string>
#include <unordered_map>

namespace kv {
	enum class CommandType {
		Set, Get, Del, Exit, Unknown, Empty
	};

	struct Command {
		CommandType type;
		std::string key;
		std::string value;
	};

	struct Result {
		bool ok;
		std::string msg;
	};

	Command parse(const std::string&);
	Result execute(const Command&, std::unordered_map<std::string, std::string>&);
}