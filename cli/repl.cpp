#include "repl.h"
#include <iostream>
#include <string>
#include <string_view>

static void print_help()
{
    std::cout << "Commands:\n"
              << "  PUT <key> <value>\n"
              << "  GET <key>\n"
              << "  DEL <key>\n"
              << "  SNAPSHOT\n"
              << "  HELP\n"
              << "  EXIT\n";
}

void run_repl(Cognex& db)
{
    std::string line;
    int write_count = 0;
    constexpr int SNAPSHOT_THRESHOLD = 5;

    std::cout << "cognex> ";

    while (std::getline(std::cin, line))
    {
        std::string_view sv(line);

        // Trim leading spaces
        while (!sv.empty() && sv.front() == ' ')
            sv.remove_prefix(1);

        if (sv.empty()) {
            std::cout << "cognex> ";
            continue;
        }

        // Parse command
        size_t p1 = sv.find(' ');
        std::string_view cmd =
            (p1 == std::string_view::npos) ? sv : sv.substr(0, p1);

        std::string_view rest =
            (p1 == std::string_view::npos) ? std::string_view{} : sv.substr(p1 + 1);

        if (cmd == "PUT")
        {
            size_t p2 = rest.find(' ');
            if (p2 == std::string_view::npos) {
                std::cout << "[ERR invalid PUT]\n";
            } else {
                std::string_view key = rest.substr(0, p2);
                std::string_view val = rest.substr(p2 + 1);

                if (key.empty() || val.empty()) {
                    std::cout << "[ERR invalid PUT]\n";
                } else {
                    db.put(
                        Key{std::string(key)},
                        Value{std::string(val)}
                    );
                    write_count++;
                    std::cout << "[Success]\n";
                }
            }
        }
        else if (cmd == "GET")
        {
            if (rest.empty()) {
                std::cout << "[ERR invalid GET]\n";
            } else {
                auto v = db.get(Key{std::string(rest)});
                if (v)
                    std::cout << v->value << "\n";
                else
                    std::cout << "[NIL]\n";
            }
        }
        else if (cmd == "DEL")
        {
            if (rest.empty()) {
                std::cout << "[ERR invalid DEL]\n";
            } else {
                if (db.del(Key{std::string(rest)})) {
                    write_count++;
                    std::cout << "[Success]\n";
                } else {
                    std::cout << "[NIL]\n";
                }
            }
        }
        else if (cmd == "SNAPSHOT")
        {
            db.snapshot();
            write_count = 0;
            std::cout << "[Success]\n";
        }
        else if (cmd == "HELP")
        {
            print_help();
        }
        else if (cmd == "EXIT")
        {
            break;
        }
        else
        {
            std::cout << "[ERR unknown command]\n";
        }

        if (write_count >= SNAPSHOT_THRESHOLD)
        {
            write_count = 0;
            db.snapshot();
            std::cout << "[auto snapshot]\n";
        }

        std::cout << "cognex> ";
    }
}
