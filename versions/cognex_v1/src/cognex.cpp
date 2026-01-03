#include "cognex.h"
#include "wal.h"   
#include <unordered_map>
#include <fstream>
#include <sstream>


static std::unordered_map<std::string,std::string> store;

//forward dec
void append_and_fsync(const std::string& path, const std::string& record);

Cognex::Cognex(const std::string& wal_path):wal_path_(wal_path){} 

void Cognex::put(const std::string& key,const std::string& value)
{
	append_and_fsync(wal_path_,"PUT " + key + " " + value + "\n");
	store[key]=value;
}

std::optional<std::string> Cognex::get(const std::string& key) const
{
	auto it = store.find(key);

	if(it == store.end())return std::nullopt;

	return it->second;
}

bool Cognex::del(const std::string& key)
{
	auto it = store.find(key);
	if(it == store.end())return false;

	append_and_fsync(wal_path_,"DEL " + key + "\n");
	store.erase(it);
	return true;
}

void Cognex::recover()
{
	std::ifstream in(wal_path_);

	if(!in.good())return;

	std::string line;

	while(std::getline(in, line))
	{
		std::istringstream iss(line);
		std::string op;
		iss>>op;
		if(op=="PUT")
		{
			std::string k,v;
			iss>>k>>v;
			if(!k.empty())store[k]=v;
		}
		else if(op == "DEL")
		{
			std::string k;
			iss>>k;
			if(!k.empty())store.erase(k);
		}
		// If a line is partial/malformed, it’s safely ignored

	}

}









