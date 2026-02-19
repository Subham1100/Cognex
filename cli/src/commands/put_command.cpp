#include "commands/put_command.h"
#include "cognex.h"
#include <iostream>

void PutCommand::execute(Cognex& db,
                         const std::vector<std::string>& args,
                         int& write_count)
{
    if (args.size() != 2) {
        std::cout << "[ERR invalid PUT]\n";
        return;
    }

    db.put(Key{args[0]}, Value{args[1]});
    write_count++;
    std::cout << "[Success]\n";
}
