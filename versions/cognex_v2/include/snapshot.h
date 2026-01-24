#pragma once
#include <string>
#include <unordered_map>

void write_snapshot_atomic(const std::string& path, const std::unordered_map<std::string,std::string>& store);
void load_snapshot(const std::string& path, std::unordered_map<std::string,std::string>& store);
