#include "cognex.h"
#include <string_view>


//-------Helpers-------------
void Cognex::apply_record_(const std::string_view& record)
{
    size_t p1 = record.find(' ');
    if (p1 == std::string_view::npos)
        return;

    std::string_view op = record.substr(0, p1);

    if (op == "PUT")
    {
        size_t p2 = record.find(' ', p1 + 1);
        if (p2 == std::string_view::npos)
            return;

        std::string_view key_sv   = record.substr(p1 + 1, p2 - (p1 + 1));
        std::string_view value_sv = record.substr(p2 + 1);

        Key key{std::string(key_sv)};
        Value value{std::string(value_sv)};

        uint64_t offset = storageEngine_.append_to_value_log_(
            static_cast<uint32_t>(key.value.size()),
            value.value
        );

         IndexEntry indexEntry {
            offset,
            static_cast<uint32_t>(value.value.size())
        };

        // push_entry_(key, value);
        indexEngine_.insert(key, value, entries_,postings_);
        index_.insert_or_assign(std::move(key), indexEntry);

        
    }
    else if (op == "DEL")
    {
        std::string_view key_sv = record.substr(p1 + 1);

        Key key{std::string(key_sv)};
        index_.erase(key);
    }
}




// void Cognex::push_entry_(Key key, Value value)
// {
// 	size_t currId = entries_.size();
// 	Entry entry {currId,key,value,generate_tokens_update_tokenIndex_(std::string_view(value.value),currId)};
//     entries_.push_back(std::move(entry));
// }


//-------------Engine-------------

// Cognex::Cognex(WalPath wal_path,SnapshotPath snapshot_path,ValueLogPath valuelog_path
// 	):wal_path_(std::move(wal_path)),snapshot_path_(std::move(snapshot_path)),valuelog_path_(std::move(valuelog_path)){}

Cognex::Cognex(WalPath wal_path,
               SnapshotPath snapshot_path,
               ValueLogPath valuelog_path)
: storageEngine_(std::move(wal_path),
                 std::move(snapshot_path),
                 std::move(valuelog_path))
{}

void Cognex::recover()
{
    storageEngine_.recover(index_,
        [this](std::string_view rec)
        {
            apply_record_(rec);
        });
}

// void Cognex::open_value_log_if_needed_()
// {
//     if (valueLogFd_ != -1)
//         return;

//     valueLogFd_ = ::open(
//         valuelog_path_.value.c_str(),
//         O_RDWR | O_CREAT | O_APPEND,
//         0644
//     );

//     if (valueLogFd_ == -1)
//         throw std::runtime_error("Failed to open values.log");
// }

// uint64_t Cognex::append_to_value_log_(uint32_t keySize, std::string_view value)
// {
//     // valueLog_.log -> header[[key_size][value_size]][value][checksum]
//     open_value_log_if_needed_();

//     off_t offset = lseek(valueLogFd_, 0, SEEK_END);
//     if (offset == -1)
//         throw std::runtime_error("lseek failed");

//     RecordHeader header {
//         keySize,
//         static_cast<uint32_t>(value.size())
//     };

//     uint32_t checksum = crc32_buf(&header, sizeof(header));
//     checksum = crc32_extend(checksum, value.data(), value.size());

//     write_all(valueLogFd_, &header, sizeof(header));
//     write_all(valueLogFd_, value.data(), value.size());
//     write_all(valueLogFd_, &checksum, sizeof(checksum));

//     valueLogWrites_++;
//     if(valueLogWrites_>=valueLogFsyncEveryNWrites_)
//     {
//          if(fsync(valueLogFd_)!=0)
//         {
//             throw std::runtime_error("fsync WAL error at append_to_value_log_");
//         }
//         valueLogWrites_=0;
//     }
    

//     return static_cast<uint64_t>(offset);
// }

// std::optional<Value> Cognex::read_from_log_(uint64_t offset, uint32_t keySize) const
// {
//     if (valueLogFd_ == -1)
//         throw std::runtime_error("Value log not open");
//     // Seek to the PROVIDED offset (not end)
//     off_t pos = ::lseek(valueLogFd_, static_cast<off_t>(offset), SEEK_SET);


//     if (pos == -1)
//         throw std::runtime_error("lseek failed");

//     RecordHeader header;
//     // Read header
//     // Use pread_all() to perform offset-based reads without modifying the
//     // shared file descriptor cursor. This prevents read/write interference.
//     pread_all(valueLogFd_, &header, sizeof(header),offset);

//     // // Validate key size
//     if (header.keySize != keySize)
//         throw std::runtime_error("Key size discrepancy detected");

//         // Read value
//     std::string value(header.valueSize, '\0');
//         // Read value at precise offset
//     pread_all(valueLogFd_,
//               value.data(),
//               header.valueSize,
//               static_cast<off_t>(offset + sizeof(header)));

//         // Read checksum
//     uint32_t storedChecksum;
//         pread_all(valueLogFd_,
//               &storedChecksum,
//               sizeof(storedChecksum),
//               static_cast<off_t>(offset + sizeof(header) + header.valueSize));

//         // Recompute checksum
//     uint32_t computedChecksum = crc32_buf(&header, sizeof(header));
//     computedChecksum = crc32_extend(computedChecksum, value.data(), value.size());

//         if (storedChecksum != computedChecksum)
//         throw std::runtime_error("Value log corruption detected (checksum mismatch)");

//      return Value{std::move(value)};
// }


void Cognex::put(Key key,Value value)
{


	// append_and_fsync(wal_path_,"PUT " + key.value + " "+ value.value, walWrites_, walFsyncEveryNWrites_);
	storageEngine_.append_wal_record("PUT " + key.value + " "+ value.value);
    // store_.insert_or_assign(
    // std::move(key),
    // std::move(value)
	// );
    uint64_t offset = storageEngine_.append_to_value_log_(static_cast<uint32_t>(key.value.size()),value.value);
    IndexEntry indexEntry {
    offset,
    static_cast<uint32_t>(value.value.size())
    };    
    
	

    // push_entry_(key, value);// push entry should go after append_to_log: if crash 
                            //after WAL the PUT will be there and no offset
                            // WAL-> push_entry (can take time) -> offset (this can be missed)
    indexEngine_.insert(key, value, entries_,postings_);

    index_.insert_or_assign(std::move(key), indexEntry);

    snapshotWriteOps_++;
    if(snapshotWriteOps_>=snapshotEveryNWriteOps_)
    {
        snapshot();
        snapshotWriteOps_=0;
    }
}

bool Cognex::del(const Key& key)
{
	//Trade off : writing a log or searching a key
	// if(!store_.count(key))return false;

	// append_and_fsync(wal_path_,"DEL " +key.value);
	// store_.erase(key);
	// return true;
    storageEngine_.append_wal_record( "DEL " + key.value);

        // if we delete bytes from valueLog_.log offset indexing will change.
     return index_.erase(key) > 0;
}

// std::vector<size_t> Cognex::query(std::string_view token) const
// {
//     std::vector<size_t> result;

//     auto it = postings_.find(std::string(token));
//     if (it == postings_.end())
//         return result;

//     for (const auto& posting : it->second)
//         result.push_back(posting.entryId);

//     return result;
// }

std::vector<QueryResult> Cognex::query(const Query& query) const
{
    return queryEngine_.execute(query, postings_);
}


std::optional<Value> Cognex::get(const Key& key) const{

	// auto it = store_.find(key);
	// if(it == store_.end()) return std::nullopt;
	// return it->second;

    auto it = index_.find(key);
    if(it == index_.end()) return std::nullopt;
    return storageEngine_.read_from_log_(it->second.offset, static_cast<uint32_t>(key.value.size()));
}

void Cognex::snapshot()
{
	storageEngine_.snapshot(index_);
}

    