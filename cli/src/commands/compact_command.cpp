#include "commands/compact_command.h"
#include "cognex.h"
#include <iostream>

void CompactCommand::execute(Cognex& db,
                         const std::vector<std::string>& /*args*/,
                         int& /*write_count*/)
{
    db.compact();
    std::cout << "[Compaction Completed]\n";
}