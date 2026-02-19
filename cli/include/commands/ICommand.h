#pragma once
#include <vector>
#include <string>

class Cognex;

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void execute(Cognex& db,
                         const std::vector<std::string>& args,
                         int& write_count) = 0;

    virtual bool is_exit() const { return false; }
};
