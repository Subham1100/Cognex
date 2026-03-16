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

    //---------------------------------
    // sortby filter
    //---------------------------------
    if (f.field == "sortby" || f.field == "sort by")
    {
        std::string val = f.value;

        std::transform(val.begin(), val.end(), val.begin(), ::tolower);

        if (val == "relevance")
            q.sortBy = SortField::RELEVANCE;

        else if (val == "similarity")
            q.sortBy = SortField::SIMILARITY;

        return;
    }
    size_t val = 0;

    try {
            val = std::stoul(f.value);
        }
    catch (...) 
        {
            return;
        }

    //---------------------------------
    // topk
    //---------------------------------
    if (f.field == "top" && f.op == "=")
        {
            q.topK = val;
        }
    //---------------------------------
    // numeric filters
    //---------------------------------
    else if (f.field == "relevance" || f.field == "similarity")
        {
            q.filters.push_back({f.field, f.op, val});
        }
}

std::vector<std::string> split_terms(const std::string& text)
{
    std::vector<std::string> terms;
    std::string current;

    for (char c : text)
    {
        if (std::isspace(c))
        {
            if (!current.empty())
            {
                terms.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty())
        terms.push_back(current);

    return terms;
}

Query parse_query(const std::vector<std::string>& args)
{
    Query q;

    if (args.empty())
        return q;

    // first argument is the search term
    // q.terms.push_back(args[0]);
    auto terms = split_terms(args[0]);
    q.terms.insert(q.terms.end(), terms.begin(), terms.end());

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

    for (const QueryResult& query_result : results)
    {
        const Entry& entry = db.get_entry(query_result.entryId);

        std::cout << "  " << query_result.entryId
                  << " key=" << entry.key.value
                  << " relevance=" << query_result.relevance
                  << " similarity=" << query_result.similarity
                  << std::endl;
    }
}