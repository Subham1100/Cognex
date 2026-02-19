#pragma once
#include "commands/ICommand.h"

class ExitCommand : public ICommand {
public:
    void execute(Cognex& db,
                 const std::vector<std::string>& args,
                 int& write_count) override {}

    bool is_exit() const override { return true; }
};
