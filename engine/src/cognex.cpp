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

        store_.insert_or_assign(std::move(key), std::move(value));
    }
    else if (op == "DEL")
    {
        std::string_view key_sv = record.substr(p1 + 1);

        Key key{std::string(key_sv)};
        store_.erase(key);
    }
}


std::vector<std::string> Cognex::generate_tokens_update_tokenIndex_(std::string_view value, size_t entryId)
{
	std::vector<std::string> tokenVector;
	size_t startPointer = 0;
	size_t valueLength = value.length();
	size_t tokenNumber = 0;
	while(startPointer<valueLength)
	{
		size_t endPointer = value.find(' ',startPointer);
		if(endPointer == std::string_view::npos)
			endPointer=value.length();
		std::string token(value.substr(startPointer, endPointer - startPointer));
        token = clean_token(std::string_view(token));

        if(!token.empty())
        {
        	
        	auto& postings = tokenIndex_[token];
        	// the idea is tokens will be aligned for all new entries_ but modification
        	// updates on PUT still is an issue.

        	if(!postings.empty() && (postings.back().entryId == entryId))
        	{
        		postings.back().frequency++;
        		postings.back().tokenPositions.push_back(tokenNumber);
        	}
        	else
        	{
        		//Posting has a constructor, so it's NOT an aggregate.
        		postings.emplace_back(entryId,1,std::vector<size_t>{tokenNumber} );
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

Cognex::Cognex(WalPath wal_path,SnapshotPath snapshot_path
	):wal_path_(std::move(wal_path)),snapshot_path_(std::move(snapshot_path)){}

void Cognex::recover()
{
    load_snapshot(snapshot_path_, store_);

    wal_replay(wal_path_,
        [this](std::string_view rec)
        {
            apply_record_(rec);
        });
}

void Cognex::put(Key key,Value value)
{
	append_and_fsync(wal_path_,"PUT " + key.value + " "+ value.value);
	push_entry_(key, value);
    store_.insert_or_assign(
    std::move(key),
    std::move(value)
	);
	
}

bool Cognex::del(const Key& key)
{
	//Trade off : writing a log or searching a key
	// if(!store_.count(key))return false;

	// append_and_fsync(wal_path_,"DEL " +key.value);
	// store_.erase(key);
	// return true;

	append_and_fsync(wal_path_, "DEL " + key.value);
    return store_.erase(key) > 0;
}

std::vector<EntryId> Cognex::query(const std::string_view& token) const
{
    std::vector<EntryId> results;

    auto it = tokenIndex_.find(std::string(token));
    if (it == tokenIndex_.end())
        return results;

    for (const auto& posting : it->second)
        results.push_back(posting.entryId);

    return results;
}


std::optional<Value> Cognex::get(const Key& key) const{

	auto it = store_.find(key);
	if(it == store_.end()) return std::nullopt;
	return it->second;
}

void Cognex::snapshot()
{
	write_snapshot_atomic(snapshot_path_,store_);
	wal_truncate(wal_path_);
}

