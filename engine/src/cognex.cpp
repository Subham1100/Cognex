#include "cognex.h"
#include "wal.h"
#include "snapshot.h"
#include "debug.h"

#include <unordered_map>
#include <string_view>

//------------Main DB----------- 
static std::unordered_map<Key,Value> store;
//------------Secondary Storages-----------
static std::vector<Entry> entries;
static std::unordered_map<std::string,std::vector<Posting>> tokenIndex;

//-------Helpers-------------
static void apply_record (const std::string_view& record)
{
	size_t p1 = record.find(' ');
	if(p1 == std::string::npos) return;

	std::string_view op = record.substr(0,p1);
	if(op == "PUT")
	{
		size_t p2 = record.find(' ',p1+1);
		if(p2==std::string_view::npos)return;

		  Key key{std::string (record.substr(p1 + 1, p2 - (p1 + 1)))};
        Value value{std::string (record.substr(p2 + 1))};
        store.insert_or_assign(
    std::move(key),
    std::move(value)
);
	}
	else if(op == "DEL")
	{
		Key key{std::string(record.substr(p1 + 1))};
        store.erase(key);
	}
}

static std::string clean_token(std::string token)
{
    // Remove leading whitespace
    size_t start = 0;
    while (start < token.size() && std::isspace(static_cast<unsigned char>(token[start])))
        ++start;

    // Remove trailing whitespace
    size_t end = token.size();
    while (end > start && std::isspace(static_cast<unsigned char>(token[end - 1])))
        --end;

    return token.substr(start, end - start);
}

static std::vector<std::string> generate_tokens_update_tokenIndex(std::string_view value, size_t entryId)
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
        token = clean_token(token);

        if(!token.empty())
        {
        	
        	auto& postings = tokenIndex[token];
        	// the idea is tokens will be aligned for all new entries but modification
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
        	debug_posting_array(postings);
        	tokenVector.push_back(std::move(token));
        	tokenNumber++;

        }
        startPointer = endPointer + 1;
	}
	
	return tokenVector;
}

static void push_entry(Key key, Value value)
{
	size_t currId = entries.size();
	Entry entry {currId,key,value,generate_tokens_update_tokenIndex(std::string_view(value.value),currId)};
    entries.push_back(std::move(entry));
}


//-------------Engine-------------

Cognex::Cognex(WalPath wal_path,SnapshotPath snapshot_path
	):wal_path_(std::move(wal_path)),snapshot_path_(std::move(snapshot_path)){}

void Cognex::recover()
{
	load_snapshot(snapshot_path_,store);
	wal_replay(wal_path_,apply_record);
}

void Cognex::put(Key key,Value value)
{
	append_and_fsync(wal_path_,"PUT " + key.value + " "+ value.value);
	push_entry(key, value);
    store.insert_or_assign(
    std::move(key),
    std::move(value)
	);
	
}

bool Cognex::del(const Key& key)
{
	//Trade off : writing a log or searching a key
	// if(!store.count(key))return false;

	// append_and_fsync(wal_path_,"DEL " +key.value);
	// store.erase(key);
	// return true;

	append_and_fsync(wal_path_, "DEL " + key.value);
    return store.erase(key) > 0;
}

std::optional<Value> Cognex::get(const Key& key) const{

	auto it = store.find(key);
	if(it == store.end()) return std::nullopt;
	return it->second;
}

void Cognex::snapshot()
{
	write_snapshot_atomic(snapshot_path_,store);
	wal_truncate(wal_path_);
}
