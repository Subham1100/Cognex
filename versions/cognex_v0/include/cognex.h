#pragma once
#include <string>
#include <optional>

class Cognex {
public:
	void put(const std::string& key, const std::string& value);
	std::optional <std::string> get(const std::string& key);
	bool del(const std::string& key);
};