#pragma once

#include "commands/ICommand.h"
#include <vector>
#include <string>

class CompactCommand : public ICommand {
public:
    void execute(Cognex& db, const std::vector<std::string>& args,int& write_count) override;
};