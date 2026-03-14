#pragma once
#include <string>
#include <unordered_map>
#include "core/types.h"

// void write_snapshot_atomic(const SnapshotPath& path, const std::unordered_map<Key,Value>& store_);
void write_snapshot_atomic(const SnapshotPath& path, const std::unordered_map<Key,IndexEntry>& index_);

// void load_snapshot(const SnapshotPath& path, std::unordered_map<Key,Value>& store_);
void load_snapshot(const SnapshotPath& path, std::unordered_map<Key,IndexEntry>& index_);

