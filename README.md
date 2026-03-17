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
  - `PUT <key> <value>`
  - `GET <key>`
  - `DEL <key>`
- **Search API**
  - Tokenizes values into terms.
  - Builds an **inverted index** from tokens → posting lists.
  - Supports `QUERY` over tokens with scoring and filtering (see `QueryEngine`).
- **Efficient reads**
  - In‑memory hash index `Key → {offset, valueSize}`.
  - Offset‑based reads from the value log using `pread`.
- **CLI / REPL**
  - Interactive prompt for issuing commands.
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

Inside the Cognex prompt:

```text
> PUT ("name") ("Alice is good") ---> entry 0
> PUT ("n") ("Bob is good") -----> entry 1
[Success]

> GET ("name")
Alice is good

> QUERY ("good")
0
1

> DEL name
[Success]

> GET name
[NIL]
```

Supported commands are documented in more detail in `docs/guide.md`.

---

## 7. Benchmarks

These results are from the `bench/bench.cpp` program using a dataset of **10,000 documents**.

### Benchmark Results

- **Dataset size**: 10,000 documents

#### PUT Benchmark (Index Build)

Measures **indexing / document insertion performance**.

| Metric              | Value              |
|---------------------|--------------------|
| Documents indexed   | 10,000             |
| Index build time    | 0.995196 sec       |
| Throughput          | 10,048 docs/sec    |

#### GET Benchmark

Measures **direct key retrieval performance**.

| Metric              | Value              |
|---------------------|--------------------|
| Operations          | 10,000             |
| Total time          | 0.0240313 sec      |
| Throughput          | 416,124 ops/sec    |

#### QUERY Benchmark

Measures **search performance over the index** using single‑term queries.

| Metric              | Value              |
|---------------------|--------------------|
| Queries             | 10,000             |
| Total time          | 4.55477 sec        |
| Throughput          | 2,195 queries/sec  |

For full methodology and notes, see `BENCHMARKS.md`.

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
  - REPL, command parsing, and command implementations (`PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `HELP`, `EXIT`).
  - `main.cpp` currently contains a commented‑out REPL entry point that wires `Cognex` to the CLI.
- **`bench/`**
  - `bench.cpp` – standalone benchmark harness used for the results above.
- **`docs/`**
  - Existing user‑facing and internals documentation.

---

## 9. Roadmap

These items are either mentioned in existing docs or implied by the code but are **not all implemented yet**:

- Typed values beyond raw strings. **TODO: design and implement.**
- Value log compaction / garbage collection. **TODO.**
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
