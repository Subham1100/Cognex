#include "cognex.h"
#include "wal.h"
#include "snapshot.h"

#include <unordered_map>
#include <string_view>

//------------Main DB----------- 
static std::unordered_map<Key,Value> store;
//------------Secondary Storages-----------
static std::vector<Entry> entries;
static std::unordered_map<std::string,std::vector<size_t>> tokenIndex;

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
