#include "cognex.h"
#include <unordered_map>


static std::unordered_map<std::string,std::string> store;

void Cognex::put(const std::string& key, const std::string& value)
{
	store[key]= value;
}

std::optional<std::string> Cognex::get(const std::string& key)
{
	auto it = store.find(key);
	if(it==store.end())return std::nullopt;
	return it->second;
}

bool Cognex::del(const std::string& key)
{
	return store.erase(key)>0;
}