## Query Engine

This document describes how queries are represented and executed in Cognex, based on `Query`, `QueryResult`, `QueryEngine`, and the `QUERY` CLI command.

---

## Query Model

The core types are defined in `core/types.h`:

- **`Filter`**
  - `std::string field`
  - `std::string op`
  - `size_t value`
- **`QueryType`**
  - `TERM`, `PHRASE`, `BOOLEAN` (**currently only term queries are clearly wired – TODO**).
- **`Ranking`**
  - `TF`, `TF_IDF`, `BM25` (**BM25 is implemented; the `ranking` field is not yet used to switch algorithms – TODO**).
- **`SortField`**
  - `RELEVANCE`, `SIMILARITY`, `DATE`, `LENGTH` (only `RELEVANCE` and `SIMILARITY` are implemented).
- **`Query`**
  - `std::vector<std::string> terms`
  - `std::vector<Filter> filters`
  - `size_t topK` (default 10)
  - Flags: `useAnd`, `useOr`, `useNot` (**present but not yet honored in `QueryEngine` – TODO**)
  - `QueryType type`
  - `Ranking ranking`
  - `SortField sortBy`
- **`QueryResult`**
  - `size_t entryId`
  - `size_t relevance` – aggregate term‑frequency score.
  - `size_t similarity` – BM25‑derived score, normalized to 0–100.
  - `double score` – additional BM25 score field (not exposed by the CLI).

---

## Execution Pipeline

The main entrypoint is:

```cpp
std::vector<QueryResult> QueryEngine::execute(
    const Query& query,
    const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_,
    size_t totalDocs,
    size_t totalTokens_,
    const std::vector<Entry>& entries_) const;
```

The execution pipeline is:

1. **Candidate generation** (`generate_candidates`)
2. **Filter application** (`apply_filters`)
3. **Ranking and top‑K selection** (`rank_results`)

### 1. Candidate Generation

`generate_candidates` iterates over query terms and postings:

- Ensures internal buffers have size at least `entries_.size()`:
  - `termFrequencyScoresBuffer_`
  - `bm25ScoresBuffer_`
- Tracks which documents are touched:
  - `touchedDocs_`
- Computes average document length:

```cpp
double avgDL = static_cast<double>(totalTokens_) / totalDocs;
```

- For each term in `query.terms`:
  - Look up its posting list in `postings_`.
  - Compute document frequency `df = postingsForTerm.size()`.
  - Compute IDF:

    ```cpp
    double idf = compute_idf(totalDocs, df);
    ```

  - For each `Posting`:
    - Let `docId = posting.entryId`.
    - If this is the first time the doc is seen, push it into `touchedDocs_`.
    - Update term‑frequency score:

      ```cpp
      termFrequencyScoresBuffer_[docId] += posting.frequency;
      ```

    - Compute document length:

      ```cpp
      size_t docLength = entries_[docId].tokens.size();
      ```

    - Compute BM25 contribution:

      ```cpp
      double bm25Contribution = compute_bm25(
          posting.frequency,
          docLength,
          avgDL,
          idf);
      ```

    - Accumulate into `bm25ScoresBuffer_[docId]`.

- After processing all terms:
  - Find `maxScore` across `touchedDocs_`.
  - For each `docId` in `touchedDocs_`:
    - Create a `QueryResult`:
      - `entryId = docId`
      - `relevance = termFrequencyScoresBuffer_[docId]`
      - `similarity = normalized BM25 score in 0–100`:

        ```cpp
        if (maxScore > 0)
            score = (bm25ScoresBuffer_[docId] / maxScore) * 100.0;
        ```
  - Reset scores in the buffers for touched docs.

### 2. Filters

`apply_filters` applies numeric filters:

- For each `QueryResult`:
  - For each `Filter` in `query.filters`:
    - Supported fields:
      - `"relevance"` → `result.relevance`
      - `"similarity"` → `result.similarity`
    - Supported operators:
      - `>`, `>=`, `<`, `<=`, `=`
  - The result is kept only if it passes all filters.

Filters are populated by the CLI `QUERY` parser (see below).

### 3. Ranking and Top‑K

`rank_results` orders and trims results:

- Comparator:
  - If `query.sortBy == SortField::SIMILARITY`:
    - Sort by `similarity` descending.
  - Otherwise (`RELEVANCE` or default):
    - Sort by `relevance` descending.
- If `query.topK >= results.size()`:
  - Simply `std::sort`.
- Else:
  - Use `std::nth_element` to partition on top‑K.
  - Resize to K.
  - Sort just the top‑K slice.

The returned vector contains the top ranked matches according to the chosen sort field.

---

## CLI Integration (`QUERY` Command)

The `QUERY` command is implemented in `cli/src/commands/query_command.cpp`.

### Argument Parsing

Given:

```text
QUERY ("alice database") ("relevance >= 2") ("top = 20") ("sortby similarity")
```

The CLI:

1. Uses the line parser to extract quoted arguments:
   - `args[0] = "alice database"`
   - `args[1] = "relevance >= 2"`
   - `args[2] = "top = 20"`
   - `args[3] = "sortby similarity"`
2. `parse_query(args)`:
   - Splits `args[0]` on whitespace into terms: `["alice", "database"]`.
   - Appends them to `Query::terms`.
   - For each additional arg:
     - `parse_filter` interprets expressions like:
       - `relevance >= 2`
       - `similarity > 50`
       - `top = 20`
       - `sortby similarity`

Supported filters:

- `top = N`
  - Sets `query.topK = N`.
- `relevance <op> N`, `similarity <op> N`
  - Adds numeric filters on the corresponding fields.
- `sortby relevance`, `sortby similarity`
  - Sets `query.sortBy`.

Unsupported / invalid expressions are ignored.

### Output

For a non‑empty result set, `QueryCommand::execute` prints:

```text
Found entries:
  <entryId> key=<key> relevance=<relevance> similarity=<similarity>
  ...
```

If no results are found, it prints:

```text
[NIL]
```

---

## Limitations and TODOs

- `QueryType`, `Ranking`, and logical flags (`useAnd`, `useOr`, `useNot`) are defined but not fully honored in `QueryEngine::execute` yet.
- Only numeric filters on `relevance` and `similarity` are implemented.
- Sort fields `DATE` and `LENGTH` are defined but have no backing metadata.
- Phrase / boolean queries and more advanced ranking modes would require additional implementation.

These areas are intentionally left as **TODOs** for future development.

