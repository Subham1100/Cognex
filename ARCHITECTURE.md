## Cognex Architecture

This document describes the core components, data structures, and algorithms used by Cognex, based on the current implementation in `engine/`, `cli/`, and `bench/`.

Where behavior is unclear or not wired up yet, it is explicitly marked as **TODO**.

---

## 1. High‑Level Design

Cognex is a log‑structured key–value store with:

- **Write‑Ahead Log (WAL)** for durability.
- **Append‑only value log** for storing values.
- **In‑memory index** `Key → {offset, valueSize}`.
- **Inverted index** for token‑based search over values.
- **Snapshots** to speed up recovery.

The main façade class is `Cognex`, which implements:

- `DB` – key–value interface (`put`, `get`, `del`).
- `Persistence` – durability interface (`recover`, `snapshot`).

`Cognex` composes three engines:

- `StorageEngine` – disk layout and durability.
- `IndexEngine` – write‑time tokenization and index construction.
- `QueryEngine` – read‑time query processing and ranking.

---

## 2. Core Modules

### 2.1 Core Types (`core/types.h`)

Key data structures:

- **Paths**
  - `WalPath`, `SnapshotPath`, `ValueLogPath`: simple wrappers around `std::string` file paths.
- **Key / Value**
  - `Key` – wrapper around `std::string` with `operator==` and `std::hash<Key>` specialization.
  - `Value` – wrapper around `std::string`.
- **Entry**
  - Represents a document in the search index:
    - `entryId`
    - `Key key`
    - `Value value`
    - `std::vector<std::string> tokens` – pre‑tokenized representation of the value.
- **Posting**
  - Inverted index posting entry:
    - `entryId` – which `Entry` contains the token.
    - `frequency` – how many times the token appears in that entry.
    - `tokenPositions` – positions of the token inside the tokenized value.
- **TransparentHash / TransparentEqual**
  - Heterogeneous lookup helpers for `std::unordered_map` keyed by `std::string` / `std::string_view`.
- **IndexEntry**
  - Value‑log index entry:
    - `offset` – byte offset in value log file.
    - `valueSize` – size of the value in bytes.
- **RecordHeader**
  - On‑disk header for each value log record:
    - `keySize`
    - `valueSize`
- **Query model**
  - `Filter` – simple numeric filters on fields like `relevance` or `similarity`.
  - `QueryType` – `TERM`, `PHRASE`, `BOOLEAN`. (**TODO: only term queries are clearly wired today.**)
  - `Ranking` – `TF`, `TF_IDF`, `BM25`. (**TODO: enum is present, BM25 is implemented, but the `ranking` field is not fully used for switching strategies.**)
  - `SortField` – `RELEVANCE`, `SIMILARITY`, `DATE`, `LENGTH`. (`DATE`/`LENGTH` are not currently backed by explicit fields.)
  - `Query` – contains:
    - `terms` – vector of token strings.
    - `filters` – vector of `Filter`.
    - `topK`, `useAnd`, `useOr`, `useNot` – **TODO: boolean flags are defined but not fully exercised in `QueryEngine`.**
    - `type`, `ranking`, `sortBy`.
  - `QueryResult`
    - `entryId`
    - `relevance` – integer score based on term frequency.
    - `similarity` – BM25‑derived score scaled to \[0, 100] and stored as `size_t`.
    - `score` – additional BM25 score field (currently unused in `QueryEngine` implementation).
  - `QueryContext`
    - Holds `const Query& query` and `std::vector<QueryResult> results`.

### 2.2 DB and Persistence Interfaces (`core/db.h`)

- `struct DB`
  - Pure virtual interface:
    - `put(Key, Value)`
    - `get(const Key&) -> std::optional<Value>`
    - `del(const Key&) -> bool`
- `struct Persistence`
  - `recover()` – restores state from disk.
  - `snapshot()` – writes a new snapshot and truncates WAL.

`class Cognex` implements both.

---

## 3. Storage Engine

### 3.1 StorageEngine (`storage/storage_engine.h/.cpp`)

Responsibilities:

- Maintain file paths and file descriptors:
  - `WalPath wal_path_`
  - `SnapshotPath snapshot_path_`
  - `ValueLogPath valuelog_path_`
  - `int valueLogFd_` – open file descriptor for value log.
- Track durability tunables:
  - `walWrites_`, `walFsyncEveryNWrites_`
  - `valueLogWrites_`, `valueLogFsyncEveryNWrites_`

#### 3.1.1 Value Log Layout

The value log is append‑only. Each record is written as:

```text
[RecordHeader { keySize, valueSize }][value bytes][checksum]
```

- **Checksum** is computed using `crc32_buf` and `crc32_extend` over header and value.

Key methods:

- `open_value_log_if_needed_()`
  - Lazily opens the value log file (`O_RDWR | O_CREAT | O_APPEND`).
- `append_to_value_log_(uint32_t keySize, std::string_view value) -> uint64_t`
  - Seeks to end of file.
  - Writes header, value, and checksum.
  - Periodically `fsync`s based on `valueLogFsyncEveryNWrites_`.
  - Returns the starting offset for the record.
- `read_from_log_(uint64_t offset, uint32_t keySize) -> std::optional<Value>`
  - Uses `lseek` + `pread_all` to:
    - Read `RecordHeader`.
    - Validate `header.keySize == keySize` (defensive check).
    - Read value bytes.
    - Read stored checksum.
    - Recompute checksum and compare.
  - Returns a `Value` on success or throws on checksum mismatch.

#### 3.1.2 WAL and Snapshots

- `append_wal_record(const std::string& record)`
  - Delegates to `append_and_fsync(wal_path_, record, walWrites_, walFsyncEveryNWrites_)`.
  - WAL records are text lines such as `"PUT <key> <value>"` or `"DEL <key>"`.
- `snapshot(const std::unordered_map<Key, IndexEntry>& index_)`
  - Calls `write_snapshot_atomic(snapshot_path_, index_)`.
  - Truncates WAL via `wal_truncate(wal_path_)`.

#### 3.1.3 Recovery

- `recover(std::unordered_map<Key,IndexEntry>& index_, std::function<void(std::string_view)> apply_record_)`
  - Opens value log.
  - Loads snapshot into `index_` using `load_snapshot`.
  - Replays WAL via `wal_replay`, invoking the `apply_record_` callback for each record.

The `apply_record_` callback is implemented in `Cognex` and is responsible for applying WAL mutations.

---

## 4. Cognex Engine

### 4.1 Cognex Class (`cognex.h/.cpp`)

`class Cognex : public DB, public Persistence` wires everything together.

State:

- `std::unordered_map<Key, IndexEntry> index_`
  - Main key index (`Key → IndexEntry{offset, valueSize}`).
- `std::vector<Entry> entries_`
  - In‑memory collection of all entries participating in search.
- `std::unordered_map<std::string, std::vector<Posting>, TransparentHash, TransparentEqual> postings_`
  - Inverted index: `token → posting list`.
- `size_t snapshotWriteOps_`
  - Counter for writes since the last snapshot.
- `size_t snapshotEveryNWriteOps_`
  - Snapshot threshold (currently `10,000,000`).
- `size_t totalTokens_`
  - Global count of tokens across all entries (used for average document length in BM25).
- Engines:
  - `QueryEngine queryEngine_`
  - `IndexEngine indexEngine_`
  - `StorageEngine storageEngine_`

#### 4.1.1 Construction

```cpp
Cognex::Cognex(WalPath wal_path,
               SnapshotPath snapshot_path,
               ValueLogPath valuelog_path)
  : storageEngine_(std::move(wal_path),
                   std::move(snapshot_path),
                   std::move(valuelog_path)) {}
```

No recovery is automatically performed at construction; callers are expected to invoke `recover()` when appropriate.

#### 4.1.2 Applying WAL Records

`apply_record_(std::string_view record)` is used during recovery:

- Parses `"PUT key value"` and `"DEL key"`.
- For `PUT`:
  - Calls `storageEngine_.append_to_value_log_` to write the value and get an offset.
  - Builds an `IndexEntry`.
  - Calls `indexEngine_.insert(key, value, entries_, postings_, totalTokens_)` to rebuild search structures.
  - Inserts/updates `index_` with the new `IndexEntry`.
- For `DEL`:
  - Erases the key from `index_`.

#### 4.1.3 `put`

```cpp
void Cognex::put(Key key, Value value)
```

Write flow:

1. Append a WAL record:
   - `storageEngine_.append_wal_record("PUT " + key.value + " " + value.value);`
2. Append to value log:
   - `offset = storageEngine_.append_to_value_log_(keySize, value.value);`
3. Create and store `IndexEntry{offset, valueSize}` in `index_`.
4. Update search structures:
   - `indexEngine_.insert(key, value, entries_, postings_, totalTokens_);`
5. Update snapshot counter and trigger snapshot if threshold is reached:
   - `snapshotWriteOps_++` and possibly call `snapshot()` (which delegates to `StorageEngine::snapshot`).

#### 4.1.4 `get`

```cpp
std::optional<Value> Cognex::get(const Key& key) const
```

Read flow:

1. Lookup key in `index_`.
2. If missing → `std::nullopt`.
3. If present → call `storageEngine_.read_from_log_(offset, keySize)` and return the resulting `Value`.

Only the value log is touched; WAL and snapshot are not involved in normal reads.

#### 4.1.5 `del`

```cpp
bool Cognex::del(const Key& key)
```

Delete flow:

1. Append `"DEL <key>"` to WAL via `append_wal_record`.
2. Remove the key from `index_`.
3. Returns `true` if a key was erased, `false` otherwise.

**Note:** deletion does not reclaim space in the value log; this would require compaction. **TODO: value log compaction / GC is not implemented.**

#### 4.1.6 `query`

```cpp
std::vector<QueryResult> Cognex::query(const Query& query) const
```

Search flow:

1. Compute `totalDocs = index_.size()`.
2. Call:
   - `queryEngine_.execute(query, postings_, totalDocs, totalTokens_, entries_);`

The query engine uses the inverted index (`postings_`) and document statistics (`entries_`, `totalTokens_`) to rank results.

#### 4.1.7 Recovery and Snapshot

- `recover()`
  - Calls `storageEngine_.recover(index_, apply_record_)`:
    - Loads snapshot into `index_`.
    - Replays WAL, using `apply_record_` to rebuild `index_`, `entries_`, and `postings_`.
  - Clears search structures:
    - `entries_.clear()`, `postings_.clear()`, `totalTokens_ = 0`.
  - Rebuilds search index from recovered `index_`:
    - Iterates over `index_` and reads each value via `storageEngine_.read_from_log_`.
    - Calls `indexEngine_.insert` for each entry.

- `snapshot()`
  - Delegates to `storageEngine_.snapshot(index_)`.

---

## 5. Index Engine

### 5.1 IndexEngine (`index/index_engine.h/.cpp`)

Responsibilities:

- Tokenize values.
- Maintain:
  - `entries_` – ordered list of `Entry`.
  - `postings_` – inverted index mapping tokens to posting lists.
  - `totalTokens_` – total number of tokens across all entries.

#### 5.1.1 Tokenization

```cpp
std::vector<std::string> IndexEngine::tokenize_and_update(
    std::string_view value,
    size_t entryId,
    std::unordered_map<std::string,
                       std::vector<Posting>,
                       TransparentHash,
                       TransparentEqual>& postings_) const;
```

Algorithm (simplified):

- Scan the value, splitting on spaces.
- For each token:
  - Normalize using `clean_token(...)` (from `core/utils.h`).  
    **TODO: see `core/utils.h` for exact normalization rules (e.g., lowercasing, punctuation stripping).**
  - Skip empty tokens.
  - For the token’s posting list:
    - If `entryId` already has a posting, increment `frequency` and append the token position.
    - Otherwise, create a new `Posting` with:
      - `entryId`
      - `frequency = 1`
      - `tokenPositions = {position}`
- Push tokens into a `std::vector<std::string>` to be stored on the `Entry`.

#### 5.1.2 Insert

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

- Computes `entryId = entries_.size()`.
- Calls `tokenize_and_update` to:
  - Update postings for all tokens in the value.
  - Return the token list.
- Increments `totalTokens_` by the number of tokens.
- Appends a new `Entry{entryId, key, value, tokens}` to `entries_`.

This function is invoked both during normal `put` operations and during recovery rebuild from `index_` and the value log.

---

## 6. Query Engine

### 6.1 QueryEngine (`query/query_engine.h/.cpp`)

The query engine executes `Query` objects against the inverted index and produces ranked `QueryResult` lists.

#### 6.1.1 BM25 and IDF

Helper functions:

- `compute_idf(totalDocs, df)`
  - `idf = log(totalDocs / df)` for `df > 0`, otherwise `0.0`.
- `compute_bm25(termFrequency, docLength, avgDL, inverseDocumentFrequency, k=1.2, b=0.75)`
  - Standard BM25 score:
    - Numerator: `tf * (k + 1)`.
    - Denominator: `tf + k * (1 - b + b * (|D| / avgDL))`.
    - Score: `idf * (numerator / denominator)`.

#### 6.1.2 Candidate Generation

```cpp
void QueryEngine::generate_candidates(
    QueryContext& ctx,
    const std::unordered_map<std::string,
                             std::vector<Posting>,
                             TransparentHash,
                             TransparentEqual>& postings_,
    size_t totalDocs,
    size_t totalTokens_,
    const std::vector<Entry>& entries_) const;
```

Key details:

- Uses mutable buffers:
  - `termFrequencyScoresBuffer_` – TF scores per document.
  - `bm25ScoresBuffer_` – BM25 scores per document.
  - `touchedDocs_` – list of doc IDs touched by the query.
- Resizes buffers to at least `entries_.size()` when needed.
- Computes:
  - `avgDL = totalTokens_ / totalDocs` (average document length).
- For each query term:
  - Looks up postings in `postings_`.
  - For each posting:
    - Marks doc as touched the first time it’s seen.
    - Accumulates term frequency in `termFrequencyScoresBuffer_`.
    - Computes BM25 contribution using:
      - `tf = posting.frequency`
      - `docLength = entries_[docId].tokens.size()`
      - `idf = compute_idf(totalDocs, documentFrequency)`
    - Accumulates into `bm25ScoresBuffer_[docId]`.
- After processing all terms:
  - Finds `maxScore` across `touchedDocs_`.
  - For each touched doc:
    - Constructs `QueryResult`:
      - `entryId`
      - `relevance = termFrequencyScoresBuffer_[docId]`
      - `similarity = normalized BM25 score in \[0, 100]` (scaled by `maxScore`).
  - Resets TF/BM25 buffers only for touched docs.

**Note:** The `Ranking` enum in `Query` is not currently used to change scoring behavior; BM25 is always used here. **TODO: honor `Query::ranking`.**

#### 6.1.3 Filters

```cpp
void QueryEngine::apply_filters(QueryContext& ctx) const;
```

- For each result:
  - Applies filters in `query.filters` sequentially.
  - Supports numeric comparisons against:
    - `"relevance"`
    - `"similarity"`
  - Operators: `>`, `>=`, `<`, `<=`, `=`.
- Keeps only results that pass all filters.

#### 6.1.4 Ranking and Top‑K

```cpp
void QueryEngine::rank_results(QueryContext& ctx) const;
```

- Comparator:
  - If `sortBy == SortField::SIMILARITY` → order by `similarity` descending.
  - Else (`SortField::RELEVANCE` or default) → order by `relevance` descending.
- If `topK >= results.size()`:
  - Fully `std::sort`.
- Else:
  - Uses `std::nth_element` to partition around top‑K.
  - Resizes to K.
  - Sorts only the top‑K slice.

There is also `apply_topk`, but `execute` currently relies on `rank_results` to enforce `topK`.

#### 6.1.5 Execute

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

- Flow:
  1. Build `QueryContext`.
  2. `generate_candidates`.
  3. `apply_filters`.
  4. `rank_results`.
  5. Return `ctx.results`.

---

## 7. CLI and Benchmarks

### 7.1 CLI (`cli/`)

The CLI code defines:

- Command interfaces and implementations (`PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `HELP`, `EXIT`).
- Parser and REPL logic (`parser`, `repl`).

`cli/main.cpp` currently contains a commented‑out `main` that:

- Computes `~/.cognex` directory paths.
- Constructs `Cognex` with `WalPath`, `SnapshotPath`, and `ValueLogPath`.
- Calls `db.recover()` and `run_repl(db)`.

This reflects the intended wiring for the CLI but may not be the active entry point depending on build configuration. **TODO: confirm the current executable wiring and update this section if needed.**

### 7.2 Benchmarks (`bench/bench.cpp`)

`bench.cpp` is a standalone binary that:

- Creates a `Cognex` instance with local files:
  - `wal.log`, `snapshot.dat`, `value.log` in the working directory.
- Runs three benchmarks with:
  - `N = 10000` (documents).
  - `QUERY_COUNT = 10000`.
- Workloads:
  - **PUT benchmark**
    - Inserts `N` keys `key0`..`keyN-1` with random “sentences” of tokens from a fixed dictionary.
  - **GET benchmark**
    - Performs `QUERY_COUNT` random point lookups on existing keys.
  - **QUERY benchmark**
    - Performs `QUERY_COUNT` single‑term queries over random tokens from the dictionary.
- Reports:
  - Elapsed time and computed throughput for each workload.
  - Approximate memory usage (Linux `/proc/self/status`).

Exact benchmark numbers and interpretation are documented in `BENCHMARKS.md`.

---

## 8. Design Decisions and Performance Characteristics

### 8.1 Design Decisions

- **Append‑only logging**
  - WAL and value log are append‑only for simpler crash recovery and sequential write performance.
- **Index in memory**
  - `index_`, `entries_`, and `postings_` are all kept in memory to keep the implementation small and fast.
  - Snapshots persist `index_` to disk; search structures are rebuilt on startup.
- **BM25‑based ranking**
  - BM25 is used to rank documents; scores are normalized and stored as `similarity` in `QueryResult`.
- **Batched fsync**
  - Both WAL and value log use counters and thresholds to avoid `fsync` on every write.
  - This trades some worst‑case durability (bounded by the batch size) for much higher throughput.

### 8.2 Performance Characteristics

From the demonstrated benchmark harness:

- **PUT**
  - Dominated by WAL appends and value log writes (including checksums and occasional `fsync`).
  - Also triggers tokenization and postings updates.
- **GET**
  - Purely in‑memory index lookup plus offset read from value log.
  - Very fast in the 10,000‑document benchmark.
- **QUERY**
  - Uses inverted index and BM25 scoring.
  - Cost depends on:
    - Number of query terms.
    - Posting list sizes for those terms.
    - Number of documents touched.

**TODO:** For more detailed performance analysis (CPU profiles, memory footprint across dataset sizes, compaction impact), additional tooling and profiling would be required.

---

## 9. Known Gaps / TODOs

- Implement value log compaction / space reclamation.
- Wire `Query::ranking`, `QueryType`, logical operators (`useAnd`, `useOr`, `useNot`) into `QueryEngine`.
- Support more sort fields (`DATE`, `LENGTH`) by enriching stored metadata.
- Confirm and document the exact CLI entry point and supported options.
- Expand tests and benchmarks to larger datasets and different workloads.

