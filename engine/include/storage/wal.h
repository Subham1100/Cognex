#pragma once 
#include <string>
#include "core/types.h"
#include "core/utils.h"



void append_and_fsync(const WalPath& path,const std::string& record,size_t& walWrites_,size_t& walFsyncEveryNWrites_);

void wal_truncate(const WalPath& path);

//-------wal_replay------

template<typename ApplyFn>

bool wal_replay(const WalPath& path, ApplyFn apply);
#include "wal.tpp"