#include "parser.h"
// #include <iostream>

// using std::cout;
std::string to_upper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}

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
    size_t p1 = sv.find('\"');
    std::string cmd = (p1 == std::string_view::npos) ? std::string(sv) : std::string(sv.substr(0, p1));

    //remove tailing whitespace
    while (!cmd.empty() && cmd.back() == ' ')
        cmd.pop_back();

    if (cmd.empty()) {
        return std::nullopt;
    }

    //add command to parsecommand {name:"cmd"}
    result.name = to_upper(cmd);

    std::string_view rest = (p1 == std::string_view::npos) ? std::string_view{} : sv.substr(p1);

    // -------- parse all quoted arguments --------
    size_t pos = 0;

    while (true)
    {
        //getting keyString (or any argument)
        size_t keyStart = rest.find('\"', pos);
        if (keyStart == std::string_view::npos)
            break;

        size_t keyEnd = rest.find('\"', keyStart + 1);
        if (keyEnd == std::string_view::npos || keyEnd <= keyStart + 1)
            break;

        std::string keyString = std::string(rest.substr(keyStart + 1, keyEnd - (keyStart + 1)));

        //push key to result args vector
        result.args.push_back(keyString);

        // cout << "Parsed arg: " << keyString << "\n";

        pos = keyEnd + 1;
    }

    // // print all arguments for verification
    // cout << "All arguments:\n";
    // for (const auto& arg : result.args)
    // {
    //     cout << arg << "\n";
    // }

    return result;
}