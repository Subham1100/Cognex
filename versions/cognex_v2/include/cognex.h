#pragma once
#include <string>
#include <optional>

class Cognex {
public:
	explicit Cognex(const std::string& wal_path,const std::string& snapshot_path);

	void put(const std::string& key, const std::string& value);
	std::optional<std::string> get(const std::string& key) const;
	bool del(const std::string& key);

	void recover();
	void snapshot();

private:
	std::string wal_path_;
	std::string snapshot_path_;
};