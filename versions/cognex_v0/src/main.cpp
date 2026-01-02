#include "cognex.h"
#include <iostream>

int main()
{
	Cognex db;
	db.put("hello","world");

	auto v = db.get("hello");

	if(v){
		std::cout<<*v<<"\n";
	}
}