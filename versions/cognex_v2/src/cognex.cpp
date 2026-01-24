#include "cognex.h"
#include "wal.h"
#include "snapshot.h"

#include <unordered_map>
#include <sstream>
#include <string_view>

static std::unordered_map<std::string,std::string> store;

static void apply_record (const std::string_view& record)
{
	size_t p1 = record.find(' ');
	if(p1 == std::string::npos) return;

	std::string_view op = record.substr(0,p1);
	if(op == "PUT")
	{
		size_t p2 = record.find(' ',p1+1);
		if(p2==std::string_view::npos)return;

		  std::string key(record.substr(p1 + 1, p2 - (p1 + 1)));
        std::string val(record.substr(p2 + 1));

		store[key] = val;
	}
	else if(op == "DEL")
	{
		std::string key(record.substr(p1 + 1));
        store.erase(key);
	}
}

Cognex::Cognex(const std::string& wal,const std::string& snap
	):wal_path_(wal),snapshot_path_(snap){}

void Cognex::recover()
{
	load_snapshot(snapshot_path_,store);
	wal_replay(wal_path_,apply_record);
}

void Cognex::put(const std::string& key,const std::string& value)
{
	append_and_fsync(wal_path_,"PUT " + key + " "+ value);
	store[key] = value;
}

bool Cognex::del(const std::string& key)
{
	if(!store.count(key))return false;


	append_and_fsync(wal_path_,"DEL " +key);
	store.erase(key);
	return true;
}

std::optional<std::string> Cognex::get(const std::string& key) const{
	auto it = store.find(key);
	if(it == store.end()) return std::nullopt;
	return it->second;
}

void Cognex::snapshot()
{
	write_snapshot_atomic(snapshot_path_,store);
	wal_truncate(wal_path_);
}
