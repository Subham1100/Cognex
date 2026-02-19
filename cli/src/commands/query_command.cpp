#include "commands/query_command.h"
#include "cognex.h"
#include <iostream>

void QueryCommand::execute(Cognex& db,
                           const std::vector<std::string>& args,
                           int& /*write_count*/)
{
    if (args.empty()) {
        std::cout << "[ERR QUERY needs arguments]\n";
        return;
    }

    std::vector<size_t> results = db.query(args[0]);

    if (results.empty()) {
        std::cout << "[NIL]\n";
        return;
    }

    std::cout << "Found entries:\n";
    for (size_t id : results)
        std::cout << "  " << id << "\n";
}

