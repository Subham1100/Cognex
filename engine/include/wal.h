#pragma once 
#include <string>
#include "types.h"



void append_and_fsync(const WalPath& path,const std::string& record);

void wal_truncate(const WalPath& path);

//-------wal_replay------

template<typename ApplyFn>

bool wal_replay(const WalPath& path, ApplyFn apply);
#include "wal.tpp"