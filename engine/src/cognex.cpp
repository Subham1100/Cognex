#include "cognex.h"
#include "wal.h"
#include "snapshot.h"
#include "debug.h"
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

        uint64_t offset = append_to_value_log_(
            static_cast<uint32_t>(key.value.size()),
            value.value
        );

         IndexEntry indexEntry {
            offset,
            static_cast<uint32_t>(value.value.size())
        };

        index_.insert_or_assign(std::move(key), indexEntry);

        push_entry_(key, value);
    }
    else if (op == "DEL")
    {
        std::string_view key_sv = record.substr(p1 + 1);

        Key key{std::string(key_sv)};
        index_.erase(key);
    }
}


std::vector<std::string> Cognex::generate_tokens_update_tokenIndex_(std::string_view value, size_t entryId)
{
	std::vector<std::string> tokenVector;
	size_t startPointer = 0;
	size_t valueLength = value.length();
	size_t tokenNumber = 0;
    std::unordered_map<std::string,size_t> currEntryIdPostingHash;

	while(startPointer<valueLength)
	{
        while (startPointer < valueLength && value[startPointer] == ' ')
        startPointer++;
        if (startPointer >= valueLength)
        break;
		size_t endPointer = value.find(' ',startPointer);
		if(endPointer == std::string_view::npos)
			endPointer=value.length();
		std::string token = clean_token(value.substr(startPointer, endPointer - startPointer));

        if(!token.empty())
        {
        	// find the token in tokenIndex
        	
            auto existInCurrEntryIdPostingHash = currEntryIdPostingHash.find(token);
            // find the existing token posting related to the entryId 
            if(existInCurrEntryIdPostingHash == currEntryIdPostingHash.end())
            {
                size_t postingIndex = postings_.size();
                postings_.emplace_back(entryId, 1, std::vector<size_t>{});
                postings_.back().tokenPositions.push_back(tokenNumber);
                currEntryIdPostingHash.emplace(token,postingIndex);
                tokenIndex_[token].emplace_back(entryId);
            }
            else 
            {
                size_t postingIndex = existInCurrEntryIdPostingHash->second;
                postings_[postingIndex].frequency++;
                postings_[postingIndex].tokenPositions.emplace_back(tokenNumber);
            }
       
        	tokenVector.push_back(std::move(token));
        	tokenNumber++;

        }
        startPointer = endPointer + 1;
	}
	
	return tokenVector;
}

void Cognex::push_entry_(Key key, Value value)
{
	size_t currId = entries_.size();
	Entry entry {currId,key,value,generate_tokens_update_tokenIndex_(std::string_view(value.value),currId)};
    entries_.push_back(std::move(entry));
}


//-------------Engine-------------

Cognex::Cognex(WalPath wal_path,SnapshotPath snapshot_path,ValueLogPath valuelog_path
	):wal_path_(std::move(wal_path)),snapshot_path_(std::move(snapshot_path)),valuelog_path_(std::move(valuelog_path)){}

void Cognex::recover()
{
    load_snapshot(snapshot_path_, index_);

    wal_replay(wal_path_,
        [this](std::string_view rec)
        {
            apply_record_(rec);
        });
}

void Cognex::open_value_log_if_needed_()
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

uint64_t Cognex::append_to_value_log_(uint32_t keySize, std::string_view value)
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

    fsync(valueLogFd_);

    return static_cast<uint64_t>(offset);
}

std::optional<Value> Cognex::read_from_log_(uint64_t offset, uint32_t keySize) const
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


void Cognex::put(Key key,Value value)
{


	append_and_fsync(wal_path_,"PUT " + key.value + " "+ value.value);
	
    // store_.insert_or_assign(
    // std::move(key),
    // std::move(value)
	// );
    uint64_t offset = append_to_value_log_(static_cast<uint32_t>(key.value.size()),value.value);
    IndexEntry indexEntry {
    offset,
    static_cast<uint32_t>(value.value.size())
    };    
    index_.insert_or_assign(key, indexEntry);
	

    push_entry_(key, value);// push entry should go after append_to_log: if crash 
                            //after WAL the PUT will be there and no offset
                            // WAL-> push_entry (can take time) -> offset (this can be missed)
    
}

bool Cognex::del(const Key& key)
{
	//Trade off : writing a log or searching a key
	// if(!store_.count(key))return false;

	// append_and_fsync(wal_path_,"DEL " +key.value);
	// store_.erase(key);
	// return true;

    append_and_fsync(wal_path_, "DEL " + key.value);

        // if we delete bytes from valueLog_.log offset indexing will change.
     return index_.erase(key) > 0;
}

const std::vector<size_t>& Cognex::query( std::string_view token) const
{
   
   static const std::vector<size_t> empty;

    auto it = tokenIndex_.find(token);

    if (it == tokenIndex_.end())
        return empty;

    return it->second;
}


std::optional<Value> Cognex::get(const Key& key) const{

	// auto it = store_.find(key);
	// if(it == store_.end()) return std::nullopt;
	// return it->second;

    auto it = index_.find(key);
    if(it == index_.end()) return std::nullopt;
    return read_from_log_(it->second.offset, static_cast<uint32_t>(key.value.size()));
}

void Cognex::snapshot()
{
	write_snapshot_atomic(snapshot_path_,index_);
	wal_truncate(wal_path_);
}

