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
                             TransparentEqual>& postings_, size_t totalDocs, size_t totalTokens_,const std::vector<Entry>& entries_) const;

private:
	void generate_candidates(
        QueryContext& ctx,
        const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_,size_t totalDocs, size_t totalTokens_,const std::vector<Entry>& entries_) const;

    void apply_filters(QueryContext& ctx) const;

    void rank_results(QueryContext& ctx) const;

    void apply_topk(QueryContext& ctx) const;

    // //------ mutables ------------------
    // mutable std::vector<size_t> termFrequencyScoresBuffer_;
    // mutable std::vector<double> bm25ScoresBuffer_;
    // mutable std::vector<size_t> touchedDocs_;

};
