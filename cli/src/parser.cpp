#include "parser.h"

std::optional<ParsedCommand> parse_line(std::string_view sv)
{
    ParsedCommand result;

// Trim leading spaces
    while (!sv.empty() && sv.front() == ' ')
            sv.remove_prefix(1);

     if (sv.empty()) {
            return std::nullopt;
        }
// Parse command
    size_t p1 = sv.find("(\"");
    std::string cmd = (p1 == std::string_view::npos) ? std::string(sv) : std::string(sv.substr(0, p1));

//remove tailing whitespace
    while (!cmd.empty() && cmd.back() == ' ')
        cmd.pop_back();


    if (cmd.empty()) {
            return std::nullopt;
        }

//add command to parsecommand {name:"cmd"}
    result.name = cmd;

    std::string_view rest = (p1 == std::string_view::npos) ? std::string_view{} : sv.substr(p1);

//getting keyString
    size_t keyStart = rest.find("(\"");
            if(keyStart==std::string_view::npos)
            {
                 return result;     
            }
            size_t keyEnd = rest.find("\")",keyStart);
            if(keyEnd==std::string_view::npos || keyEnd <= keyStart + 1)
            {
                 return result;
                 
            }
            std::string keyString = std::string(rest.substr(keyStart + 2 , keyEnd- (keyStart+2)));

//push key to result args vector

            result.args.push_back(keyString);

            size_t valueStart = rest.find("(\"",keyEnd);
            if(valueStart==std::string_view::npos)
            {
                 return result;
            }

            size_t valueEnd = rest.find("\")",valueStart);
            if(valueEnd==std::string_view::npos || valueEnd <= valueStart + 1)
            {
                 
                 return result;
            }

//push value to result args vector

            std::string valueString = std::string(rest.substr(valueStart + 2,valueEnd - (valueStart + 2)));

            result.args.push_back(valueString);

        return result;
}
