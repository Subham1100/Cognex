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
        // std::cout<<sv<<"\n";
        // Parse command
        size_t p1 = sv.find("(\"");
        std::string_view cmd =
            (p1 == std::string_view::npos) ? sv : sv.substr(0, p1);

        while (!cmd.empty() && cmd.back() == ' ')
            cmd.remove_suffix(1);

         if (cmd.empty()) {
            std::cout << "cognex> ";
            continue;
        }

        
        std::string_view rest =
            (p1 == std::string_view::npos) ? std::string_view{} : sv.substr(p1);

            if (cmd.empty()) {
            std::cout << "cognex> ";
            continue;
        }

        if (cmd == "PUT")
        {
            size_t keyStart = rest.find("(\"");
            if(keyStart==std::string_view::npos)
            {
                 std::cout << "[ERR invalid PUT]\n";
                 std::cout << "cognex> ";
                 continue;
                 
            }
            std::cout<<rest[keyStart]<<"\n";
            size_t keyEnd = rest.find("\")",keyStart);
            if(keyEnd==std::string_view::npos || keyEnd <= keyStart + 1)
            {
                 std::cout << "[ERR invalid PUT]\n";
                 std::cout << "cognex> ";
                 continue;
                 
            }
            std::cout<<rest[keyEnd]<<"\n";
            std::string_view keyString = rest.substr(keyStart + 2 , keyEnd- (keyStart+2));
            std::cout<<keyString<<"\n";

            size_t valueStart = rest.find("(\"",keyEnd);
            if(valueStart==std::string_view::npos)
            {
                 std::cout << "[ERR invalid PUT]\n";
                 std::cout << "cognex> ";
                 continue;
                 
            }

            size_t valueEnd = rest.find("\")",valueStart);
            if(valueEnd==std::string_view::npos || valueEnd <= valueStart + 1)
            {
                 std::cout << "[ERR invalid PUT]\n";
                 std::cout << "cognex> ";
                 continue;
                 
            }
            std::string_view valueString = rest.substr(valueStart + 2,valueEnd - (valueStart + 2));
            std::cout<<valueString<<"\n";
            if (keyString.empty() || valueString.empty()) {
                    std::cout << "[ERR invalid PUT]\n";
                    std::cout << "cognex> ";
                    continue;
                    
                } else {
                    
                    db.put(
                        Key{std::string(keyString)},
                        Value{std::string(valueString)}
                    );
                    write_count++;
                    std::cout << "[Success]\n";
                }
            
        }
        else if (cmd == "GET")
        {
            size_t keyStart = rest.find("(\"");
            if(keyStart==std::string_view::npos)
            {
                 std::cout << "[ERR invalid GET]\n";
                 std::cout << "cognex> ";
                 continue;
                 
            }
            size_t keyEnd = rest.find("\")",keyStart);
            if(keyEnd==std::string_view::npos)
            {
                 std::cout << "[ERR invalid GET]\n";
                 std::cout << "cognex> ";
                 continue;
                 
            }
            std::string_view keyString = rest.substr(keyStart+2, keyEnd- (keyStart+2));
            if (keyString.empty()) {
                std::cout << "[ERR invalid GET]\n";
                std::cout << "cognex> ";
                continue;
                
            } else {
                auto v = db.get(Key{std::string(keyString)});
                if (v){
                        std::cout << v->value << "\n";
                    }
                else{
                    std::cout << "[NIL]\n";
                }
            }
        }
        else if (cmd == "DEL")
        {
            size_t keyStart = rest.find("(\"");
            if(keyStart==std::string_view::npos)
            {
                 std::cout << "[ERR invalid DEL]\n";
                 std::cout << "cognex> ";
                 continue;
            }
            size_t keyEnd = rest.find("\")",keyStart);
            if(keyEnd==std::string_view::npos)
            {
                 std::cout << "[ERR invalid DEL]\n";
                 std::cout << "cognex> ";
                 continue;
            }
            std::string_view keyString = rest.substr(keyStart+1, keyEnd- (keyStart+1));
            if (keyString.empty()) {
                std::cout << "[ERR invalid DEL]\n";
                std::cout << "cognex> ";
                continue;
                
            } else {
                if (db.del(Key{std::string(keyString)})) {
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
            std::cout << "cognex> ";
            continue;
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
