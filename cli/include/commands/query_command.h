#pragma once
#include "commands/ICommand.h"

class QueryCommand : public ICommand {
public:
    void execute(Cognex& db,
                 const std::vector<std::string>& args,
                 int& write_count) override;
};
