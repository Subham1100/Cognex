#include "commands/query_command.h"
#include "cognex.h"
#include <iostream>
#include <algorithm>

// IMPORTANT: longest operators must appear first
const std::vector<std::string> OPERATORS = {
    ">=",
    "<=",
    "!=",
    ">",
    "<",
    "="
};
void print_query(const Query& q)
{
    std::cout << "----- Parsed Query -----\n";

    std::cout << "Terms:\n";
    for (const auto& term : q.terms)
    {
        std::cout << "  " << term << "\n";
    }

    std::cout << "Filters:\n";
    if (q.filters.empty())
    {
        std::cout << "  (none)\n";
    }
    else
    {
        for (const auto& f : q.filters)
        {
            std::cout << "  " << f.field << " " << f.op << " " << f.value << "\n";
        }
    }

    std::cout << "Top K: " << q.topK << "\n";

    std::cout << "Use AND: " << (q.useAnd ? "true" : "false") << "\n";
    std::cout << "Use OR: " << (q.useOr ? "true" : "false") << "\n";

    std::cout << "------------------------\n";
}

FilterExpr parse_expression(std::string_view expr)
{
    // trim leading spaces
    while (!expr.empty() && expr.front() == ' ')
        expr.remove_prefix(1);

    // trim trailing spaces
    while (!expr.empty() && expr.back() == ' ')
        expr.remove_suffix(1);

    for (const auto& op : OPERATORS)
    {
        size_t pos = expr.find(op);

        if (pos != std::string_view::npos)
        {
            std::string field = std::string(expr.substr(0, pos));
            std::string value = std::string(expr.substr(pos + op.size()));

            // trim trailing spaces from field
            while (!field.empty() && field.back() == ' ')
                field.pop_back();

            // trim leading spaces from value
            size_t start = 0;
            while (start < value.size() && value[start] == ' ')
                start++;

            value = value.substr(start);

            return {field, op, value};
        }
    }

    return {"", "", ""};
}

void parse_filter(const std::string& expr, Query& q)
{
    FilterExpr f = parse_expression(expr);

    if (f.field.empty() || f.value.empty())
        return;

    // normalize field to lowercase
    std::transform(f.field.begin(), f.field.end(), f.field.begin(), ::tolower);

    size_t val = 0;

    try {
        val = std::stoul(f.value);
    }
    catch (...) {
        return;
    }

    if (f.field == "top" && f.op == "=")
{
    q.topK = val;
}
else if (f.field == "relevance" || f.field == "similarity")
{
    q.filters.push_back({f.field, f.op, val});
}
}

Query parse_query(const std::vector<std::string>& args)
{
    Query q;

    if (args.empty())
        return q;

    // first argument is the search term
    q.terms.push_back(args[0]);

    // remaining arguments are filters
    for (size_t i = 1; i < args.size(); i++)
    {
        parse_filter(args[i], q);
    }

    return q;
}

void QueryCommand::execute(Cognex& db,
                           const std::vector<std::string>& args,
                           int& /*write_count*/)
{
    if (args.empty())
    {
        std::cout << "[ERR QUERY needs arguments]\n";
        return;
    }

    Query q = parse_query(args);
    // print_query(q);

    const std::vector<QueryResult> results = db.query(q);

    if (results.empty())
    {
        std::cout << "[NIL]\n";
        return;
    }

    std::cout << "Found entries:\n";

    for (QueryResult query_result : results)
        std::cout << "  " << query_result.entryId << "\n";
}