#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "commands/ICommand.h"
#include <iostream>

class CommandRegistry {
    std::unordered_map<std::string, std::unique_ptr<ICommand>> commands;
    class UnknownCommand : public ICommand {
        void execute(Cognex&, const std::vector<std::string>&, int&) override {
            std::cout << "[ERR unknown command]\n";
        }
    } unknown; //create an obj unknown with execute func so that print error.

public:
    CommandRegistry();

    ICommand& resolve(const std::string& name);
};
