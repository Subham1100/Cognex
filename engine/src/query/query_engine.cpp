#include "query/query_engine.h"
#include <unordered_map>
#include <algorithm>
#include <iostream>


// IDF Helper Function - How rare a word is across all documents.
double compute_idf(size_t totalDocs, size_t df)
{
    return std::log((totalDocs - df + 0.5) / (df + 0.5) + 1.0);
}


//---------------------------------------------
// BM25 formula components
//---------------------------------------------

double compute_bm25(
    size_t termFrequency,
    size_t docLength,
    double avgDL,
    double inverseDocumentFrequency,
    double k = 1.2,
    double b = 0.75)
{
    // numerator: TF * (k + 1)
    double numerator = termFrequency * (k + 1);

    // denominator: TF + k * (1 - b + b * (|D| / avgDL))
    // (|D| / avgDL) normalizes document length
    double denominator =
        termFrequency +
        k * (1 - b + b * ((double)docLength / avgDL));

    return inverseDocumentFrequency * (numerator / denominator);
}


void QueryEngine::generate_candidates(
    QueryContext& ctx,
    const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_,
    size_t totalDocs,
    size_t totalTokens_,
    const std::vector<Entry>& entries_) const
{
    // for a query with multiple terms, "apple" "banana"
    // if a id contain both the terms it will be added twice with frequency 1
    // so we store a temp map to mark frequency of a entryId with terms in the args

    if (ctx.termFrequencyScoresBuffer_.size() < entries_.size())
    {
    // raw term-frequency based relevance score (used for filters / legacy ranking)
        ctx.termFrequencyScoresBuffer_.resize(entries_.size(), 0);
    // similarity score using BM25 ranking
        ctx.bm25ScoresBuffer_.resize(entries_.size(), 0.0);
    }

    ctx.touchedDocs_.clear();


    // Track only documents touched by the query
    ctx.touchedDocs_.reserve(1024);

    ctx.results.reserve(1024);

    // BM25 hyperparameters 
    const double k = 1.2;//Controls TF saturation
    const double b = 0.75; //Controls length normalisation

    // average document length across corpus
    double avgDL = (double)totalTokens_ / totalDocs;

    for (const auto& term : ctx.query.terms)
    {
        auto postingsIt = postings_.find(term);
        if (postingsIt == postings_.end())
            continue;
        
        const auto& postingsForTerm = postingsIt->second;

        // DF = number of documents containing the term
        size_t documentFrequency = postingsForTerm.size();

        // IDF measures rarity of term in corpus
        double inverseDocumentFrequency = compute_idf(totalDocs, documentFrequency);

        // we goto each entryId that has the term
        // then we generate BM25 for 
        // then we store it in BM25Scores[entryId] -> BM25contribution
        for (const auto& posting : postingsForTerm)
        {
            //skip the deleted entry
            const Entry& entry = entries_[posting.entryId];
            if (entry.isDeleted) continue;

            size_t docId = posting.entryId;

            // First time we see this doc → record it
            if (ctx.termFrequencyScoresBuffer_[docId] == 0)
                ctx.touchedDocs_.push_back(docId);

            // TF = how many times term appears in document
            size_t termFrequency = posting.frequency;

            ctx.termFrequencyScoresBuffer_[docId] += termFrequency;

            // |D| = length of document in tokens
            // used by BM25 to penalize long documents
            // for ex :
            // 1 occurrence → important
            // 5 occurrences → still important
            // 50 occurrences → not 10x more important
            size_t docLength = entries_[docId].tokens.size();

            // compute BM25 score for this term in this document
            double bm25Contribution = compute_bm25(
                termFrequency,
                docLength,
                avgDL,
                inverseDocumentFrequency);

            // accumulate similarity score
            ctx.bm25ScoresBuffer_[docId] += bm25Contribution;
        }
    }

    //get maxScore to normalize BM25 scores.
    // Find max BM25 score only among touched docs
    double maxScore = 0.0;

    for (size_t docId : ctx.touchedDocs_)
    {
        double score = ctx.bm25ScoresBuffer_[docId];
        if (score > maxScore)
            maxScore = score;
    }

    // convert temporary score maps into QueryResult objects
    for (size_t docId : ctx.touchedDocs_)
    {
        QueryResult result;

        result.entryId = docId;

        // raw TF relevance (useful for filters like relevance > X)
        result.relevance = ctx.termFrequencyScoresBuffer_[docId];

        double score = ctx.bm25ScoresBuffer_[docId];
        if (maxScore > 0)
        score = (score / maxScore) * 100.0;

        // final ranking score (BM25 similarity)
        result.similarity = static_cast<size_t>(std::round(score));

        ctx.results.push_back(result);
    }

    // Reset buffers only for touched docs
    for (size_t docId : ctx.touchedDocs_)
    {
        ctx.termFrequencyScoresBuffer_[docId] = 0;
        ctx.bm25ScoresBuffer_[docId] = 0.0;
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
    auto comp = [&](const QueryResult& a, const QueryResult& b)
    {
        switch (ctx.query.sortBy)
        {
            case SortField::SIMILARITY:
                return a.similarity > b.similarity;

            case SortField::RELEVANCE:
            default:
                return a.relevance > b.relevance;
        }
    };

    size_t k = ctx.query.topK;

    if (k >= ctx.results.size())
    {
        std::sort(ctx.results.begin(), ctx.results.end(), comp);
        return;
    }

    // Partition so topK elements are first
    std::nth_element(
        ctx.results.begin(),
        ctx.results.begin() + k,
        ctx.results.end(),
        comp
    );

    // Remove everything after topK
    ctx.results.resize(k);

    // Sort only the topK results
    std::sort(ctx.results.begin(), ctx.results.end(), comp);
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
                             TransparentEqual>& postings_,size_t totalDocs, size_t totalTokens_, const std::vector<Entry>& entries_) const
{
    QueryContext ctx{query};
    generate_candidates(ctx, postings_,totalDocs, totalTokens_,entries_);
    apply_filters(ctx);
    rank_results(ctx);
    // apply_topk(ctx);

    return ctx.results;
}

