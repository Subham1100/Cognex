#include "storage/storage_engine.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

StorageEngine::StorageEngine(WalPath wal,
                             SnapshotPath snapshot,
                             ValueLogPath valueLog)
    : wal_path_(std::move(wal)),
      snapshot_path_(std::move(snapshot)),
      valuelog_path_(std::move(valueLog))
{}

void StorageEngine::recover(std::unordered_map<Key,IndexEntry>& index_,
                            std::function<void(std::string_view)> apply_record_)
{
    // open value log before reading anything from it
    open_value_log_if_needed_();

    load_snapshot(snapshot_path_, index_);

    wal_replay(wal_path_,
        [&](std::string_view rec)
        {
            apply_record_(rec);
        });
}

void StorageEngine::open_value_log_if_needed_()
{
    if (valueLogFd_ != -1)
        return;

    valueLogFd_ = ::open(
        valuelog_path_.value.c_str(),
        O_RDWR | O_CREAT | O_APPEND,
        0644
    );

    if (valueLogFd_ == -1)
        throw std::runtime_error("Failed to open values.log");
}

uint64_t StorageEngine::append_to_value_log_(uint32_t keySize, std::string_view value)
{
    // valueLog_.log -> header[[key_size][value_size]][value][checksum]
    open_value_log_if_needed_();

    off_t offset = lseek(valueLogFd_, 0, SEEK_END);
    if (offset == -1)
        throw std::runtime_error("lseek failed");

    RecordHeader header {
        keySize,
        static_cast<uint32_t>(value.size())
    };

    uint32_t checksum = crc32_buf(&header, sizeof(header));
    checksum = crc32_extend(checksum, value.data(), value.size());

    write_all(valueLogFd_, &header, sizeof(header));
    write_all(valueLogFd_, value.data(), value.size());
    write_all(valueLogFd_, &checksum, sizeof(checksum));

    valueLogWrites_++;
    if(valueLogWrites_>=valueLogFsyncEveryNWrites_)
    {
         if(fsync(valueLogFd_)!=0)
        {
            throw std::runtime_error("fsync WAL error at append_to_value_log_");
        }
        valueLogWrites_=0;
    }
    

    return static_cast<uint64_t>(offset);
}

std::optional<Value> StorageEngine::read_from_log_(uint64_t offset, uint32_t keySize) const
{
    if (valueLogFd_ == -1)
        throw std::runtime_error("Value log not open");
    // Seek to the PROVIDED offset (not end)
    off_t pos = ::lseek(valueLogFd_, static_cast<off_t>(offset), SEEK_SET);


    if (pos == -1)
        throw std::runtime_error("lseek failed");

    RecordHeader header;
    // Read header
    // Use pread_all() to perform offset-based reads without modifying the
    // shared file descriptor cursor. This prevents read/write interference.
    pread_all(valueLogFd_, &header, sizeof(header),offset);

    // // Validate key size
    if (header.keySize != keySize)
        throw std::runtime_error("Key size discrepancy detected");

        // Read value
    std::string value(header.valueSize, '\0');
        // Read value at precise offset
    pread_all(valueLogFd_,
              value.data(),
              header.valueSize,
              static_cast<off_t>(offset + sizeof(header)));

        // Read checksum
    uint32_t storedChecksum;
        pread_all(valueLogFd_,
              &storedChecksum,
              sizeof(storedChecksum),
              static_cast<off_t>(offset + sizeof(header) + header.valueSize));

        // Recompute checksum
    uint32_t computedChecksum = crc32_buf(&header, sizeof(header));
    computedChecksum = crc32_extend(computedChecksum, value.data(), value.size());

        if (storedChecksum != computedChecksum)
        throw std::runtime_error("Value log corruption detected (checksum mismatch)");

     return Value{std::move(value)};
}

void StorageEngine::append_wal_record(const std::string& record)
{
    append_and_fsync(wal_path_,record, walWrites_, walFsyncEveryNWrites_);

}

void StorageEngine::snapshot(const std::unordered_map<Key,IndexEntry>& index_)
{
    
    write_snapshot_atomic(snapshot_path_, index_);

    wal_truncate(wal_path_);
}