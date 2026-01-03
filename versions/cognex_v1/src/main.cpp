#include "cognex.h"
#include <iostream>

int main()
{
	Cognex db("data/wal.log");

	db.recover();

	db.put("name","cognex");
	db.put("version","1");

	auto v = db.get("name");
	if(v)std::cout<<*v<<"\n";
}