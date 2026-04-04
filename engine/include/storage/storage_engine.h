#pragma once

#include "core/types.h"
#include "storage/wal.h"
#include "storage/snapshot.h"
#include "debug/debug.h"
#include <optional>
#include <string_view>
#include <unordered_map>
#include <functional>

class StorageEngine
{
public:

    StorageEngine(WalPath wal,
                  SnapshotPath snapshot,
                  ValueLogPath valueLog);

    void recover(std::unordered_map<Key,IndexEntry>& index_,
                 std::function<void(std::string_view)> apply_record_);

    uint64_t append_to_value_log_(uint32_t keySize,
                                  std::string_view value);

    std::optional<Value> read_from_log_(uint64_t offset,
                                        uint32_t keySize) const;

    void snapshot(const std::unordered_map<Key,IndexEntry>& index_);

    void append_wal_record(const std::string& record);

private:

    void open_value_log_if_needed_();

    WalPath wal_path_;
    SnapshotPath snapshot_path_;
    ValueLogPath valuelog_path_;

        //------ file ------
    int valueLogFd_ = -1;

    // //------ state vairables-----
    size_t walWrites_ = 0;
    size_t walFsyncEveryNWrites_ = 1000;

    size_t valueLogWrites_ = 0;
    size_t valueLogFsyncEveryNWrites_ = 5000;

    // size_t snapshotWriteOps_ = 0;
    // size_t snapshotEveryNWriteOps_ = 10000000;

};