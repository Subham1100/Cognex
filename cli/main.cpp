#include "repl.h"
#include "cognex.h"
#include <filesystem>
#include <cstdlib>
#include <stdexcept>


int main()
{
	const char* home = std::getenv("HOME");
	if(!home)
	{
		throw std::runtime_error("HOME environment variable not set");
	}

	std::filesystem::path base = std::filesystem::path(home) / ".cognex";
	
	std::filesystem::create_directories(base);
	Cognex db(WalPath{(base/"wal.log").string()},SnapshotPath{(base/"snapshot.dat").string()},ValueLogPath{(base/"value.log")});
	
	db.recover();
	run_repl(db);
	return 0;
}