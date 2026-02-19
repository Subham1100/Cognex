#include "commands/get_command.h"
#include "cognex.h"
#include <iostream>

void GetCommand::execute(Cognex& db,
                         const std::vector<std::string>& args,
                         int& /*write_count*/) // rename unused variable
{
    if (args.size() != 1) {
        std::cout << "[ERR invalid GET]\n";
        return;
    }

    auto v = db.get(Key{args[0]});
    if (v) std::cout << v->value << "\n";
    else   std::cout << "[NIL]\n";
}
