#include "repl.h"
#include "parser.h"
#include "command_registry.h"
#include "cognex.h"
#include <iostream>

static void print_prompt() {
    std::cout << "cognex> ";
}

void run_repl(Cognex& db)
{
    std::string line;
    int write_count = 0;
    constexpr int SNAPSHOT_THRESHOLD = 5;

    CommandRegistry registry;

    print_prompt();

    while (std::getline(std::cin, line))
    {
        auto parsed = parse_line(line);

        if (!parsed) {
            std::cout << "[ERR parse error]\n";
            print_prompt();
            continue;
        }

        ICommand& cmd = registry.resolve(parsed->name);
        cmd.execute(db, parsed->args, write_count);

        if (cmd.is_exit())
            break;

        if (write_count >= SNAPSHOT_THRESHOLD)
        {
            db.snapshot();
            write_count = 0;
            std::cout << "[auto snapshot]\n";
        }

        print_prompt();
    }
}
