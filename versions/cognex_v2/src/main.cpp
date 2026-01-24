#include "cognex.h"
#include <iostream>

int main()
{
    Cognex db("../data/wal.log","../data/snapshot.dat");
    db.recover();

    db.put("name","cognex");
    db.put("version","2");

    db.snapshot();

    auto v = db.get("version");
    if(v)std::cout<<*v<<"\n";

    return 0;
}
