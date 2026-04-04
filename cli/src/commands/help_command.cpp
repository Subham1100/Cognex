#include "commands/help_command.h"
#include <iostream>

void HelpCommand::execute(Cognex&,
                          const std::vector<std::string>&,
                          int&)
{
    std::cout << "Commands:\n"
              << "  PUT \"key\" \"value\"\n"
              << "  GET \"key\"\n"
              << "  DEL \"key\"\n"
              << "  QUERY \"terms\" [filters...]\n"
              << "  SNAPSHOT\n"
              << "  COMPACT\n"
              << "  HELP\n"
              << "  EXIT\n";
}
