#pragma once 
#include <string>

void append_and_fsync(const std::string& path,const std::string& record);

bool wal_replay(const std::string& path, void(*apply)(const std::string_view& record) );

void wal_truncate(const std::string& path);