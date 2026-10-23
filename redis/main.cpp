#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include "Command.h"
#include "Store.h"



int main() {
	std::string command;

	std::cout << "Welcome to TNRC, Totally Not Redis Clone" << std::endl;

	std::cout << ">> ";
	while (std::getline(std::cin, command)) {
		kv::Command c = kv::parse(command);
		if (c.type == kv::CommandType::Exit) {
			break;
		}
		auto [ok, msg] = kv::execute(c, kv::memory_map);
		std::cout << (ok ? "" : "ERR ") << msg;
		std::cout << ">> ";
	}
}
