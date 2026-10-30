#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "Command.h"

namespace kv {
	static std::vector<std::string> split_input(const std::string& command) {
		std::vector<std::string> output;
		std::istringstream iss(command);
		std::string token;
		while (iss >> token) {
			output.push_back(token);
		}

		return output;
	}

	Command parse(const std::string& line) {
		auto input = split_input(line);
		if (input.empty()) return Command{ .type = CommandType::Empty };

		if (input[0] == "SET") {
			if (input.size() == 3) {
				return Command{ .type = CommandType::Set, .key = input[1], .value = input[2] };
			}
			else {
				throw ParseError{ "Wrong number of arguments" };
			}
		}

		if (input[0] == "GET") {
			if (input.size() == 2) {
				return Command{ .type = CommandType::Get, .key = input[1] };
			}
			else {
				throw ParseError{ "Wrong number of arguments" };
			}
		}

		if (input[0] == "DEL") {
			if (input.size() == 2) {
				return Command{ .type = CommandType::Del, .key = input[1] };
			}
			else {
				throw ParseError{ "Wrong number of arguments" };
			}
		}

		if (input[0] == "EXIT") {
			if (input.size() == 1) {
				return Command{ .type = CommandType::Exit };
			}
			else {
				throw ParseError{ "Wrong number of arguments" };
			}
		}

		return Command{ .type = CommandType::Unknown };
	}

	Result execute(const Command& command, std::unordered_map<std::string, std::string>& map) {
		if (command.type == CommandType::Set) {
			map[command.key] = command.value;
			return Result{ .ok = true, .msg = "OK\n" };
		}

		if (command.type == CommandType::Get) {
			if (const auto it = map.find(command.key); it != map.end()) {
				return Result{ .ok = true, .msg = it->second + '\n' };
			}
			return Result{ .ok = true, .msg = "(nil)\n" };
		}

		if (command.type == CommandType::Del) {
			if (const auto it = map.find(command.key); it != map.end()) {
				map.erase(it);
				return Result{ .ok = true, .msg = "OK\n" };
			}
			return Result{ .ok = true, .msg = "(nil)\n" };
		}

		if (command.type == CommandType::Empty) {
			return Result{ .ok = true, .msg = "" };
		}

		return Result{ .ok = false, .msg = "Unknown command\n" };
	}
}