#include "command_registry.h"
#include "commands/put_command.h"
#include "commands/get_command.h"
#include "commands/del_command.h"
#include "commands/snapshot_command.h"
#include "commands/help_command.h"
#include "commands/exit_command.h"
#include "commands/query_command.h"

CommandRegistry::CommandRegistry()
{
    commands["PUT"] = std::make_unique<PutCommand>();
    commands["GET"] = std::make_unique<GetCommand>();
    commands["DEL"] = std::make_unique<DelCommand>();
    commands["SNAPSHOT"] = std::make_unique<SnapshotCommand>();
    commands["HELP"] = std::make_unique<HelpCommand>();
    commands["EXIT"] = std::make_unique<ExitCommand>();
    commands["QUERY"] = std::make_unique<QueryCommand>();
}

ICommand& CommandRegistry::resolve(const std::string& name)
{
    auto it = commands.find(name);
    return (it != commands.end()) ? *it->second : unknown;
}
