## Benchmarks

This document describes the benchmark harness in `bench/bench.cpp` and the results for a 10,000‑document workload.

All numbers below are taken from the specified code and the provided measurements; environment‑specific details (CPU, RAM, disk type) are not encoded in the codebase and are therefore omitted.  
**TODO:** capture and record hardware / OS details alongside benchmark runs.

---

## 1. Benchmark Harness Overview

The benchmark binary (`bench/bench.cpp`) exercises the `Cognex` engine directly:

- Constructs a `Cognex` instance with:
  - `WalPath{"wal.log"}`
  - `SnapshotPath{"snapshot.dat"}`
  - `ValueLogPath{"value.log"}`
- Uses:
  - `N = 10000` documents.
  - `QUERY_COUNT = 10000` operations for GET and QUERY benchmarks.
- Generates random values using a fixed in‑memory dictionary:
  - Example terms: `apple`, `banana`, `database`, `engine`, `search`, `wal`, `performance`, `benchmark`, etc.
  - `random_sentence` picks 3–12 words per document, joined with spaces.
- Measures latency with `std::chrono::high_resolution_clock`.
- Reports:
  - Total time in seconds.
  - Throughput computed as `operations / seconds`.
- Approximates memory usage (on Linux) by reading `/proc/self/status` and extracting `VmRSS`.

Three workloads are measured:

1. **PUT benchmark** – `benchmark_put`
2. **GET benchmark** – `benchmark_get`
3. **QUERY benchmark** – `benchmark_query`

---

## 2. Workloads

### 2.1 PUT Benchmark – Index Build

Function: `benchmark_put(Cognex& db)`

- For `i` from `0` to `N - 1`:
  - Key: `"key" + std::to_string(i)`
  - Value: random sentence from the dictionary (3–12 tokens).
- Operations:
  - `db.put(Key{...}, Value{...})`
- Measures:
  - Time to index `N` documents.
  - Throughput in docs/sec.

What this measures:

- End‑to‑end **write path performance**, including:
  - WAL append (with batched `fsync`).
  - Value log append (header + value + checksum).
  - In‑memory index update (`index_`).
  - Tokenization and inverted index updates via `IndexEngine`.

### 2.2 GET Benchmark – Point Lookups

Function: `benchmark_get(Cognex& db)`

- Uses a uniform distribution over `0..N-1`:
  - Picks random keys of the form `"key" + k`.
- For `i` from `0` to `QUERY_COUNT - 1`:
  - Calls `db.get(Key{...})`.
- Discards the value (only measures latency).

What this measures:

- **Direct key retrieval performance**:
  - Hash map lookup in `index_`.
  - Offset‑based read from the value log using `StorageEngine::read_from_log_`.

### 2.3 QUERY Benchmark – Token Search

Function: `benchmark_query(Cognex& db)`

- Picks random tokens from the same dictionary used for values.
- For each of `QUERY_COUNT` iterations:
  - Builds a `Query`:
    - Single term in `q.terms`.
    - `topK = 10`.
  - Calls `db.query(q)`.
- Discards results after issuing the query.

What this measures:

- **Search performance over the inverted index**, including:
  - Posting list lookups (`postings_`).
  - Candidate generation and BM25 scoring.
  - Filtering (if filters are set on the query; the benchmark uses only the default).
  - Ranking and top‑K selection.

---

## 3. Results (10,000 Documents)

Dataset size: **10,000 documents**  
All results below are for single‑threaded execution of the benchmark harness.

### 3.1 Summary Table

| Benchmark | Operations / Documents | Total Time (sec) | Throughput              | What it measures                                   |
|----------:|------------------------|------------------|-------------------------|----------------------------------------------------|
| **PUT**   | 10,000 documents       | 0.995196         | 10,048 docs/sec         | Indexing / document insertion performance          |
| **GET**   | 10,000 operations      | 0.0240313        | 416,124 ops/sec         | Direct key retrieval performance                   |
| **QUERY** | 10,000 queries         | 4.55477          | 2,195 queries/sec       | Search performance over the inverted index         |

### 3.2 PUT Benchmark Details

- **Documents indexed**: 10,000  
- **Index build time**: 0.995196 sec  
- **Throughput**: 10,048 docs/sec  

Interpretation:

- Reflects the combined cost of:
  - WAL logging and periodic `fsync`.
  - Value log appends with checksums.
  - Tokenization and updates to `entries_` and `postings_`.
- For larger datasets or different hardware, throughput will vary; the code does not embed environment info.

### 3.3 GET Benchmark Details

- **Operations**: 10,000  
- **Total time**: 0.0240313 sec  
- **Throughput**: 416,124 ops/sec  

Interpretation:

- Shows that point lookups are effectively bound by:
  - Hash map lookup in `index_`.
  - A single `pread` of the value record.
- No additional work is performed in the search index for this workload.

### 3.4 QUERY Benchmark Details

- **Queries**: 10,000  
- **Total time**: 4.55477 sec  
- **Throughput**: 2,195 queries/sec  

Interpretation:

- Each query:
  - Touches the postings list for a random token.
  - Accumulates BM25 scores across all matching documents.
  - Normalizes and ranks results, then returns top‑K (10).
- Performance depends on:
  - Token distribution in generated documents.
  - Size of postings lists for frequently occurring terms.

---

## 4. How to Reproduce

Build and run the benchmark binary (exact commands depend on your CMake configuration).

Example outline:

```bash
# From the project root (exact commands may vary)
cmake -S . -B build
cmake --build build --config Release

# Then run the benchmark executable produced from bench/bench.cpp
./build/bench   # TODO: replace with the actual binary name / path
```

**TODO:** document the exact CMake target name for the benchmark once it is standardized in `CMakeLists.txt`.

The benchmark will:

- Create / reuse `wal.log`, `snapshot.dat`, and `value.log` in the working directory.
- Print PUT, GET, and QUERY statistics.
- Print an approximate memory usage in MB at the end (Linux only, via `/proc/self/status`).

---

## 5. Future Benchmarking Work

The current harness focuses on a simple, single‑threaded scenario.  
Potential extensions (**not implemented yet**):

- Vary dataset sizes (e.g., 100k, 1M, 10M documents).
- Separate warm‑cache vs. cold‑cache GET benchmarks.
- Add mixed workloads (e.g., combined PUT/GET/QUERY ratios).
- Run under different durability settings (tuning WAL / value log fsync frequencies).
- Capture detailed environment metadata with each run (CPU model, cores, disk type, OS).

