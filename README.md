## Cognex

Lightweight persistent key–value and search database written in modern C++.

Cognex is designed to explore database internals — durability, crash recovery, storage engine mechanics, and full‑text search — in a minimal but practical implementation.

---

## 1. Project Title

**Cognex – Log‑structured key–value store with inverted index search**

---

## 2. Problem Statement

**Goal:** provide a simple, educational storage engine that:

- Persists key–value data safely across crashes.
- Recovers state using WAL + snapshots.
- Supports efficient point lookups (`GET`) and token‑based search (`QUERY`).
- Exposes a small CLI so you can interactively inspect how a real storage engine behaves.

---

## 3. Key Features

- **Durable storage**
  - Write‑Ahead Log (WAL) for crash safety.
  - Append‑only value log with checksummed records.
  - Periodic snapshots of the key index to bound recovery time.
- **Key–value API**
  - `PUT "key" "value"`
  - `GET "key"`
  - `DEL "key"`
- **Search API**
  - Tokenizes values into terms.
  - Builds an **inverted index** from tokens → posting lists.
  - Supports `QUERY` over tokens with scoring and filtering (see `QueryEngine`).
- **Efficient reads**
  - In‑memory hash index `Key → {offset, valueSize}`.
  - Offset‑based reads from the value log using `pread`.
- **Search index compaction**
  - After enough deletes, or via the `COMPACT` command, Cognex drops tombstoned entries and rebuilds `entries_` / `postings_` with fresh sequential entry IDs (see `Cognex::compact()`).
  - The on-disk value log is still append-only; dead records are not removed from `value.log` yet.
- **CLI / REPL**
  - Interactive prompt for issuing commands (`PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `COMPACT`, `HELP`, `EXIT`).
- **Crash recovery**
  - Snapshot load → WAL replay → re‑index for search.

For deeper internals, see `ARCHITECTURE.md` and the `docs/` directory.

---

## 4. Architecture Overview

At a high level:

- **`Cognex` (engine)**
  - Implements the `DB` and `Persistence` interfaces.
  - Coordinates `StorageEngine`, `IndexEngine`, and `QueryEngine`.
- **`StorageEngine`**
  - Manages WAL (`wal.log`), snapshots (`snapshot.dat`), and value log (`value.log`).
  - Provides `recover`, `append_wal_record`, `append_to_value_log_`, `read_from_log_`, and `snapshot`.
- **`IndexEngine`**
  - Tokenizes document values and builds:
    - `entries_` – `std::vector<Entry>` with key, value, and tokens.
    - `postings_` – inverted index (`token → std::vector<Posting>`).
- **`QueryEngine`**
  - Executes `Query` objects using postings and BM25‑style scoring.
  - Applies filters and ranking, then returns `std::vector<QueryResult>`.

Details and diagrams are in `ARCHITECTURE.md`.

---

## 5. Installation

### Option 1: One‑line install (recommended)

```bash
curl -fsSL https://raw.githubusercontent.com/Subham1100/Cognex/main/install.sh | bash
```

Run the CLI with:

```bash
./bin/Cognex
```

### Option 2: Install from source (manual)

```bash
git clone https://github.com/Subham1100/Cognex.git
cd Cognex
chmod +x install.sh
./install.sh
```

Run the CLI with:

```bash
./bin/Cognex
```

**Build requirements**

- **CMake** ≥ 3.16  
- **C++17** compatible compiler (e.g. gcc, clang)

---

## 6. Usage Example

Inside the Cognex prompt (arguments must be double-quoted):

```text
cognex> PUT "name" "Alice is good"
[Success]

cognex> PUT "n" "Bob is good"
[Success]

cognex> GET "name"
Alice is good

cognex> QUERY "good"
Found entries:
  0 key=name relevance=... similarity=...
  1 key=n relevance=... similarity=...

cognex> DEL "name"
[Success]

cognex> GET "name"
[NIL]
```

Supported commands are documented in more detail in `docs/guide.md`.

---

## 7. Benchmarks

Results below are from the **`cognex_bench`** binary (`bench/bench.cpp`): **10,000** documents, **10,000** GETs, **10,000** single-term queries, seed **42**, fresh files under **`bench_data/`**, then `recover()`. Wall time per phase (`high_resolution_clock`), throughput = ops ÷ time. **Sample run** — your machine will differ; see `BENCHMARKS.md` for full criteria and how to reproduce.

### Benchmark Results

- **Dataset size**: 10,000 documents  
- **Approximate RSS after run**: 9 MB (harness-reported)

#### PUT Benchmark (Index Build)

| Metric              | Value              |
|---------------------|--------------------|
| Documents indexed   | 10,000             |
| Index build time    | 0.527247 s         |
| Throughput          | ~18,966 docs/s     |
| Share of phase time | 6.65% of PUT+GET+QUERY |

#### GET Benchmark

| Metric              | Value              |
|---------------------|--------------------|
| Operations          | 10,000             |
| Total time          | 0.025308 s         |
| Throughput          | ~395,138 ops/s     |
| Share of phase time | 0.32% of PUT+GET+QUERY |

#### QUERY Benchmark

| Metric              | Value              |
|---------------------|--------------------|
| Queries             | 10,000             |
| Total time          | 7.375877 s         |
| Throughput          | ~1,356 queries/s   |
| Share of phase time | 93.03% of PUT+GET+QUERY |

For methodology, criteria, and `./build/cognex_bench` usage, see `BENCHMARKS.md`.

---

## 8. Project Structure

High‑level layout:

- **`engine/`**
  - `include/core/` – core types (`Key`, `Value`, `Entry`, `Posting`, `Query`, `QueryResult`, `QueryContext`, etc.).
  - `include/storage/` – storage abstractions (`StorageEngine`, WAL, snapshot, value log).
  - `include/index/` – `IndexEngine` (tokenization + inverted index).
  - `include/query/` – `QueryEngine` (BM25 scoring, filters, ranking).
  - `include/debug/` – debug and helper utilities.
  - `src/` – implementations for the above plus `cognex.cpp`.
- **`cli/`**
  - REPL, line parser (double-quoted arguments), and command implementations (`PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `COMPACT`, `HELP`, `EXIT`).
  - `main.cpp` opens `~/.cognex/` for `wal.log`, `snapshot.dat`, and `value.log`, calls `recover()`, then runs the REPL.
- **`bench/`**
  - `bench.cpp` – standalone benchmark harness used for the results above.
- **`docs/`**
  - Existing user‑facing and internals documentation.

---

## 9. Roadmap

These items are either mentioned in existing docs or implied by the code but are **not all implemented yet**:

- Typed values beyond raw strings. **TODO: design and implement.**
- On-disk **value log** compaction / garbage collection (reclaim space in `value.log` for deleted or overwritten values). **TODO.** In-memory search structures can be compacted today; see `COMPACT` and `Cognex::compact()`.
- Concurrency control and multi‑threaded access. **TODO.**
- Transactions / MVCC semantics. **TODO.**
- Compression of on‑disk structures. **TODO.**
- More expressive query language (phrase / boolean queries using `QueryType`). **TODO: flesh out and hook into `QueryEngine`.**

Please check the issue tracker for up‑to‑date status.

---

## 10. Contributing

- **Report bugs** and **request features** via GitHub issues.
- **Send pull requests** for:
  - Documentation improvements.
  - Small, focused fixes to storage, indexing, or query logic.
  - Benchmarks or reproducible performance investigations.
- Keep changes consistent with the existing style and avoid introducing new dependencies without discussion.

By contributing, you agree that your contributions will be licensed under the project’s **MIT License**.

---

## License

MIT License
