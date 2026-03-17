## Benchmarks

This document describes the benchmark harness and results for Cognex, based on `bench/bench.cpp`.

All numbers below correspond to the 10,000‑document workload currently implemented in the repository.  
Environment details (CPU, disk, OS) are not encoded in the code and therefore omitted.  
**TODO:** Record hardware/OS information alongside future benchmark runs.

---

## 1. Harness Overview

The benchmark executable in `bench/bench.cpp`:

- Constructs a `Cognex` instance backed by local files:
  - `wal.log`
  - `snapshot.dat`
  - `value.log`
- Uses constants:
  - `N = 10000` – number of documents.
  - `QUERY_COUNT = 10000` – number of GET/QUERY operations.
- Generates random values:
  - Uses a fixed dictionary of tokens (e.g. `apple`, `database`, `engine`, `search`, `wal`, `performance`, etc.).
  - `random_sentence` composes 3–12 random tokens per value.
- Measures elapsed time with `std::chrono::high_resolution_clock`.
- Computes throughput as `operations / seconds`.
- Reads approximate memory usage (on Linux) from `/proc/self/status` (VmRSS).

Workloads:

1. `benchmark_put` – index build (PUT).
2. `benchmark_get` – random point lookups (GET).
3. `benchmark_query` – random token queries (QUERY).

---

## 2. Workloads

### 2.1 PUT Benchmark (Index Build)

Function: `benchmark_put(Cognex& db)`

- Inserts `N` key–value pairs:
  - Key: `"key" + std::to_string(i)` for `i` in `0..N-1`.
  - Value: random sentence from the dictionary.
- For each document:
  - `db.put(Key{...}, Value{...})` is called.

Measures:

- Time to index 10,000 documents.
- Throughput in documents per second.

What this exercises:

- WAL appends and batched `fsync`.
- Value‑log appends (header + value + checksum).
- In‑memory index maintenance.
- Tokenization and inverted‑index updates via `IndexEngine`.

### 2.2 GET Benchmark

Function: `benchmark_get(Cognex& db)`

- Uses a uniform distribution over `0..N-1` to select keys.
- For each of `QUERY_COUNT` iterations:
  - Picks a random key `"key" + k`.
  - Calls `db.get(Key{...})`.
  - Discards the result (only latency is measured).

Measures:

- Time for 10,000 random point lookups.
- Throughput in operations per second.

What this exercises:

- Hash‑map lookup in the in‑memory key index.
- Offset‑based read from the value log via `StorageEngine::read_from_log_`.

### 2.3 QUERY Benchmark

Function: `benchmark_query(Cognex& db)`

- For each of `QUERY_COUNT` iterations:
  - Chooses a random token from the dictionary.
  - Constructs a `Query`:
    - Single term in `q.terms`.
    - `q.topK = 10`.
  - Calls `db.query(q)`.
  - Discards results.

Measures:

- Time for 10,000 random token queries.
- Throughput in queries per second.

What this exercises:

- Inverted index lookups for the chosen token.
- BM25‑style scoring across matching documents.
- Ranking and top‑K selection in `QueryEngine`.

---

## 3. Results (10,000 Documents)

Dataset size: **10,000 documents**  
All workloads are single‑threaded.

### 3.1 Summary

| Benchmark | Operations / Documents | Total Time (sec) | Throughput             | Primary Measurement                                   |
|----------:|------------------------|------------------|------------------------|-------------------------------------------------------|
| **PUT**   | 10,000 documents       | 0.995196         | 10,048 docs/sec        | Indexing / document insertion performance             |
| **GET**   | 10,000 operations      | 0.0240313        | 416,124 ops/sec        | Direct key retrieval performance                      |
| **QUERY** | 10,000 queries         | 4.55477          | 2,195 queries/sec      | Search performance over the inverted index            |

These values should match the numbers produced by `bench/bench.cpp` when run under similar conditions.

---

### 3.2 PUT – Index Build

- **Documents indexed**: 10,000  
- **Index build time**: 0.995196 sec  
- **Throughput**: 10,048 docs/sec  

Interpretation:

- Reflects the combined overhead of:
  - WAL logging and batched `fsync`.
  - Value‑log writes with checksums.
  - Index and inverted‑index maintenance.

---

### 3.3 GET – Point Lookups

- **Operations**: 10,000  
- **Total time**: 0.0240313 sec  
- **Throughput**: 416,124 ops/sec  

Interpretation:

- Highlights efficiency of:
  - In‑memory hash index lookups.
  - Single offset‑based value‑log reads.

---

### 3.4 QUERY – Token Search

- **Queries**: 10,000  
- **Total time**: 4.55477 sec  
- **Throughput**: 2,195 queries/sec  

Interpretation:

- Cost includes:
  - Accessing the posting list for a random token.
  - Computing BM25 scores using document lengths and term frequencies.
  - Applying ranking and limiting results to top‑K.

---

## 4. How to Run

Exact build target names may vary depending on the CMake configuration. The general pattern is:

```bash
cmake -S . -B build
cmake --build build --config Release

# TODO: replace with the actual benchmark binary name/path
./build/bench
```

Running the benchmark will:

- Create or reuse `wal.log`, `snapshot.dat`, and `value.log` in the working directory.
- Print PUT, GET, and QUERY statistics.
- Print approximate memory usage in MB at the end (Linux only).

**TODO:** Document the exact benchmark target name once it is standardized in `CMakeLists.txt`.

---

## 5. Future Benchmarking Directions

The current benchmark focuses on a single, fixed workload. Potential extensions (**not implemented**):

- Larger datasets (e.g. 100k, 1M, 10M documents).
- Mixed workloads (e.g. combinations of PUT/GET/QUERY).
- Warm‑cache vs cold‑cache GET benchmarks.
- Multi‑threaded or concurrent client scenarios.
- Different durability settings (varying WAL/value‑log fsync thresholds).

Any such changes should be reflected here once implemented.

