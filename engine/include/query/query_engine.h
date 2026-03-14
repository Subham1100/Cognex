#pragma once

#include "core/types.h"
#include <unordered_map>
#include <vector>
#include <string>

class QueryEngine {
public:
	    std::vector<QueryResult> execute(
    const Query& query,
    const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_) const;

private:
	void generate_candidates(
        QueryContext& ctx,
        const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_) const;

    void apply_filters(QueryContext& ctx) const;

    void rank_results(QueryContext& ctx) const;

    void apply_topk(QueryContext& ctx) const;

};
