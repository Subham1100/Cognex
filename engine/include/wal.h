#pragma once 
#include <string>
#include "types.h"

void append_and_fsync(const WalPath& path,const std::string& record);

bool wal_replay(const WalPath& path, void(*apply)(const std::string_view& record) );

void wal_truncate(const WalPath& path);