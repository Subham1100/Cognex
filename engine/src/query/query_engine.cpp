#include "query/query_engine.h"
#include <unordered_map>
#include <algorithm>

void QueryEngine::generate_candidates(QueryContext& ctx, const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_) const
{
    // for a query with multiple terms, "apple" "banana"
    // if a id contain both the terms it will be added twice with frequency 1
    // so we store a temp map to mark frequency of a entryId with terms in the args
    std::unordered_map<size_t, size_t> scores;

    for (const auto& term : ctx.query.terms)
    {
        auto it = postings_.find(term);
        if (it == postings_.end())
            continue;

        for (const auto& posting : it->second)
        {
            scores[posting.entryId] += posting.frequency;
        }
    }

    for (auto& [entryId, relevance] : scores)
    {
        QueryResult r;
        r.entryId = entryId;
        r.relevance = relevance;

        ctx.results.push_back(r);
    }
}

void QueryEngine::apply_filters(QueryContext& ctx) const
{
    std::vector<QueryResult> filtered;

    for (const auto& r : ctx.results)
    {
        bool pass = true;

        for (const auto& f : ctx.query.filters)
        {
            size_t fieldValue = 0;

            if (f.field == "relevance")
                fieldValue = r.relevance;

            else if (f.field == "similarity")
                fieldValue = r.similarity;

            else
                continue;

            if (f.op == ">" && !(fieldValue > f.value))
                pass = false;

            else if (f.op == ">=" && !(fieldValue >= f.value))
                pass = false;

            else if (f.op == "<" && !(fieldValue < f.value))
                pass = false;

            else if (f.op == "<=" && !(fieldValue <= f.value))
                pass = false;

            else if (f.op == "=" && !(fieldValue == f.value))
                pass = false;

            if (!pass)
                break;
        }

        if (pass)
            filtered.push_back(r);
    }

    ctx.results.swap(filtered);
}

void QueryEngine::rank_results(QueryContext& ctx) const
{
    std::sort(ctx.results.begin(), ctx.results.end(),
        [](const QueryResult& a, const QueryResult& b)
        {
            return a.relevance > b.relevance;
        });
}

void QueryEngine::apply_topk(QueryContext& ctx) const
{
    if (ctx.results.size() > ctx.query.topK)
        ctx.results.resize(ctx.query.topK);
}


std::vector<QueryResult> QueryEngine::execute(
    const Query& query,
    const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_) const
{
    QueryContext ctx{query};

    generate_candidates(ctx, postings_);
    apply_filters(ctx);
    rank_results(ctx);
    apply_topk(ctx);

    return ctx.results;
}

