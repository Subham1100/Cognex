#pragma once
#include <string>
#include <optional>

class Cognex {
public:
	explicit Cognex(const std::string& wal_path);

	void put(const std::string& key, const std::string& value);
	std::optional<std::string> get(const std::string& key) const;
	bool del(const std::string& key);

	void recover();

private:
	std::string wal_path_;
};