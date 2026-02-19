#include "commands/del_command.h"
#include "cognex.h"
#include <iostream>

void DelCommand::execute(Cognex& db,
                         const std::vector<std::string>& args,
                         int& write_count)
{
    if (args.size() != 1) {
        std::cout << "[ERR invalid DEL]\n";
        return;
    }

    if (db.del(Key{args[0]})) {
        write_count++;
        std::cout << "[Success]\n";
    } else {
        std::cout << "[NIL]\n";
    }
}
