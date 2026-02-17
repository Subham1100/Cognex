#pragma once
#include <string>
#include <unordered_map>
#include "types.h"

void write_snapshot_atomic(const SnapshotPath& path, const std::unordered_map<Key,Value>& store_);
void load_snapshot(const SnapshotPath& path, std::unordered_map<Key,Value>& store_);
