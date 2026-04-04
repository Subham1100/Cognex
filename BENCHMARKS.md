## Benchmarks

This document describes the **`cognex_bench`** harness (`bench/bench.cpp`), how results are produced, and a **sample run** on one machine. Your numbers will differ by CPU, disk, and OS load.

---

## 1. How we benchmark (methodology and criteria)

### 1.1 Executable and data layout

- Build target: **`cognex_bench`** (see root `CMakeLists.txt`).
- Persistence files live under a dedicated directory (default **`bench_data/`** relative to the process working directory):
  - `wal.log`, `snapshot.dat`, `value.log`
- By default the harness **deletes** those three files before each run, then constructs `Cognex` and calls **`recover()`** so the run starts from an empty on-disk state (no stale WAL/snapshot skew).

### 1.2 Parameters (defaults)

| Parameter | Default | Meaning |
|-----------|---------|---------|
| `docs` | `10000` | Keys `key0` … `key{docs-1}` inserted in PUT phase |
| `query_count` | `10000` | Number of GET operations and number of QUERY operations |
| `seed` | `42` | `std::mt19937` seed for reproducible synthetic values and random key/query choices |
| `data_dir` | `bench_data` | Directory for WAL / snapshot / value log |

CLI: `./build/cognex_bench [docs] [query_count] [seed] [data_dir]`

### 1.3 What is measured (criteria)

- **Clock:** wall time via `std::chrono::high_resolution_clock` around each phase (not CPU time, not per-op latency percentiles).
- **Throughput:** `operations / elapsed_seconds` for that phase only.
- **Time split:** each phase’s wall time as a percentage of **PUT + GET + QUERY** total for that run (shows where end-to-end time goes in this harness).
- **Memory:** approximate **resident set size** after all phases — Linux: `VmRSS` from `/proc/self/status` (reported in MB); macOS: `mach_task_basic_info.resident_size` (reported in MB). This is a rough footprint hint, not a rigorous allocator profile.

### 1.4 What is *not* measured

- Multi-threaded or concurrent access.
- CLI parsing, network, or mixed interleaved workloads.
- Cold vs warm OS page cache (unless you change reuse of `data_dir` or run order).
- Statistical distribution across many runs (single run unless you script repeats).

### 1.5 Synthetic workload definition

- **Dictionary:** fixed token list in `bench/bench.cpp` (e.g. `apple`, `database`, `engine`, `search`, `wal`, `snapshot`, `performance`, `benchmark`, …).
- **Values:** each PUT value is **`random_sentence`**: 3–12 tokens chosen uniformly from the dictionary, space-separated.
- **PUT:** for `i ∈ [0, docs)`, `db.put(Key{"key" + std::to_string(i)}, Value{random_sentence(rng)})`.
- **GET:** `query_count` times, uniform random `k ∈ [0, docs-1)`, `db.get(Key{"key" + std::to_string(k)})`, result discarded.
- **QUERY:** `query_count` times, uniform random token from dictionary, `Query` with that single term and **`topK = 10`**, `db.query(q)`, results discarded.

Criteria in plain language: we measure **single-threaded bulk insert**, then **random point reads** on an existing corpus, then **single-term search** with fixed top-K, all on **synthetic text** with a small vocabulary (posting lists can grow large for common tokens).

---

## 2. Workloads (what each phase stresses)

### 2.1 PUT — index / write path

- WAL append (batched `fsync` in engine code).
- Value log append (header + value + checksum).
- `index_` update; `IndexEngine` tokenization and `postings_` updates.

### 2.2 GET — point lookup path

- Hash lookup in `index_`.
- Offset-based read from value log + checksum verification.

### 2.3 QUERY — search path

- Posting list lookup for the term.
- BM25-style scoring over matching documents, normalize, rank, top-K.

---

## 3. Sample results (10,000 documents, 10,000 GET, 10,000 QUERY)

Recorded from **`./build/cognex_bench`** with defaults (`docs=10000`, `query_count=10000`, `seed=42`, `dir=bench_data`).  
**Environment was not captured in the tool** — treat as one reference point, not a guarantee.

### 3.1 Summary table

| Benchmark | Operations / documents | Total time (s) | Throughput | Time split (of PUT+GET+QUERY) |
|-----------|-------------------------|----------------|------------|-------------------------------|
| **PUT**   | 10,000 documents        | 0.527247       | 18,966 docs/s | 6.65%                      |
| **GET**   | 10,000 operations       | 0.025308       | 395,138 ops/s | 0.32%                      |
| **QUERY** | 10,000 queries          | 7.375877       | 1,356 queries/s | 93.03%                  |

**End-to-end wall time (three phases):** ~7.93 s  
**Approximate RSS after run:** 9 MB  

### 3.2 Interpretation

- **QUERY dominates** this harness (~93% of phase time): posting traversal + BM25 + ranking on a 10k-doc corpus with a small dictionary drives cost.
- **GET** remains cheap: mostly index lookup + one value read per op.
- **PUT** sits in between: durability + indexing per document.

---

## 4. How to reproduce

```bash
cmake -S . -B build
cmake --build build --target cognex_bench --config Release
./build/cognex_bench
```

Custom size / seed / directory:

```bash
./build/cognex_bench 50000 50000 42 bench_run2
```

---

## 5. Future benchmarking work

- Record **CPU model, core count, RAM, disk type, OS** next to each published table.
- Multiple runs with **median / p95** wall time.
- Larger `docs`, mixed PUT/GET/QUERY ratios, optional reuse of `data_dir` for warm-cache GET.
- Tune WAL / value-log `fsync` thresholds and re-benchmark.
