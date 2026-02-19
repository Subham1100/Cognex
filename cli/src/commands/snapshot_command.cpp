#include "commands/snapshot_command.h"
#include "cognex.h"
#include <iostream>

void SnapshotCommand::execute(Cognex& db,
                              const std::vector<std::string>& /*args*/,
                              int& write_count) // rename unused args
{
    db.snapshot();
    write_count = 0;

    std::cout << "[Success]\n";
}
