## Indexing

This document describes how Cognex tokenizes values and builds its inverted index, based on `IndexEngine` and related types.

---

## Components

Indexing is handled primarily by:

- `IndexEngine` (`index/index_engine.h/.cpp`)
- Core types from `core/types.h`:
  - `Entry`
  - `Posting`
  - `TransparentHash` / `TransparentEqual`

The main in‑memory structures maintained by `Cognex` are:

- `std::vector<Entry> entries_`
- `std::unordered_map<std::string,
                      std::vector<Posting>,
                      TransparentHash,
                      TransparentEqual> postings_`
- `size_t totalTokens_`

---

## Data Structures

### Entry

Each stored document is represented as:

- `entryId` – index within `entries_`.
- `Key key` – original key.
- `Value value` – original value string.
- `std::vector<std::string> tokens` – normalized token sequence extracted from `value`.

### Posting

Each posting entry stores metadata for a specific token–document pair:

- `entryId` – ID of the `Entry` where the token appears.
- `frequency` – number of times the token appears in that entry.
- `tokenPositions` – positions (0‑based) of the token in the token sequence for that entry.

### Inverted Index

The inverted index maps:

```cpp
token (std::string) → std::vector<Posting>
```

Heterogeneous lookup (`std::string` / `std::string_view`) is supported using `TransparentHash` and `TransparentEqual`.

---

## Tokenization

Tokenization is implemented by `IndexEngine::tokenize_and_update`:

```cpp
std::vector<std::string> tokenize_and_update(
    std::string_view value,
    size_t entryId,
    std::unordered_map<std::string,
                       std::vector<Posting>,
                       TransparentHash,
                       TransparentEqual>& postings_) const;
```

Algorithm (derived from the code):

1. Iterate over `value`, splitting on spaces.
2. For each candidate token:
   - Normalize it via `clean_token(std::string_view)`.
     - **TODO:** See `core/utils.h` and implementation for exact normalization rules (case‑folding, punctuation handling, etc.).
   - Skip tokens that become empty after cleaning.
3. For each non‑empty token:
   - Look up or create its posting list: `auto& postingList = postings_[token];`
   - Search for an existing `Posting` with the same `entryId`:
     - If found:
       - Increment `frequency`.
       - Append the current token position to `tokenPositions`.
     - If not found:
       - Create a new `Posting(entryId, 1, {tokenPosition})`.
4. Append the (possibly cleaned) token string to the result vector.

The returned vector of tokens is stored on the `Entry`.

---

## Insert Flow

The main insertion API in `IndexEngine` is:

```cpp
void IndexEngine::insert(
    Key key,
    Value value,
    std::vector<Entry>& entries_,
    std::unordered_map<std::string,
                       std::vector<Posting>,
                       TransparentHash,
                       TransparentEqual>& postings_,
    size_t& totalTokens_) const;
```

Steps:

1. Compute `entryId = entries_.size()`.
2. Call `tokenize_and_update(value.value, entryId, postings_)` to:
   - Update the inverted index.
   - Get the list of tokens for this entry.
3. Increment `totalTokens_` by `tokens.size()`.
4. Append a new `Entry(entryId, key, value, std::move(tokens))` to `entries_`.

This function is used:

- On normal writes (`Cognex::put`).
- During recovery, when `Cognex::recover` iterates over the restored key index and re‑indexes documents using values read from the value log.

---

## Relationship to Queries

The inverted index and tokenized entries are consumed by `QueryEngine`:

- Query terms are matched against the keys of `postings_`.
- For each term, the corresponding posting list provides:
  - Which documents contain the term.
  - How many times it appears (for term frequency).
  - Token positions (for potential advanced features such as phrase queries – **TODO: not implemented yet**).

See `query-engine.md` for how postings are turned into rankings and `QueryResult` objects.

