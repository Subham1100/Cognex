#include "repl.h"
#include "cognex.h"

int main()
{
	Cognex db(WalPath{"../data/wal.log"},SnapshotPath{"../data/snapshot.dat"});
	db.recover();

	run_repl(db);
	return 0;
}