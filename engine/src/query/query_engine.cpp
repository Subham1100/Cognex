#include "query/query_engine.h"
#include <unordered_map>
#include <algorithm>
#include <iostream>


// IDF Helper Function - How rare a word is across all documents.
double compute_idf(size_t totalDocs, size_t df)
{
    if (df == 0) return 0.0;
    return std::log((double)totalDocs / df);
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

    // raw term-frequency based relevance score (used for filters / legacy ranking)
    std::unordered_map<size_t, size_t> termFrequencyScores;

    // similarity score using BM25 ranking
    std::unordered_map<size_t, double> bm25Scores;

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
            size_t docId = posting.entryId;

            // TF = how many times term appears in document
            size_t termFrequency = posting.frequency;

            termFrequencyScores[docId] += termFrequency;

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
            bm25Scores[docId] += bm25Contribution;
        }
    }

    //get maxScore to normalize BM25 scores.
    //
    double maxScore = 0.0;

    for (const auto& [docId, score] : bm25Scores)
    {
        if (score > maxScore)
            maxScore = score;
    }

    // convert temporary score maps into QueryResult objects
    for (auto& [docId, tfScore] : termFrequencyScores)
    {
        QueryResult result;

        result.entryId = docId;

        // raw TF relevance (useful for filters like relevance > X)
        result.relevance = tfScore;

        double score = bm25Scores[docId];
        if (maxScore > 0)
        score = (score / maxScore) * 100.0;

        // final ranking score (BM25 similarity)
        result.similarity = static_cast<size_t>(std::round(score));

        ctx.results.push_back(result);
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
                             TransparentEqual>& postings_,size_t totalDocs, size_t totalTokens_, const std::vector<Entry>& entries_) const
{
    QueryContext ctx{query};
    generate_candidates(ctx, postings_,totalDocs, totalTokens_,entries_);
    apply_filters(ctx);
    rank_results(ctx);
    apply_topk(ctx);

    return ctx.results;
}

