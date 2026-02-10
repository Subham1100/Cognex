#include "repl.h"
#include "cognex.h"
#include <filesystem>


int main()
{
	std::filesystem::create_directories("cognex-data");

	Cognex db(WalPath{"cognex-data/data/wal.log"},SnapshotPath{"cognex-data/data/snapshot.dat"});
	db.recover();

	run_repl(db);
	return 0;
}