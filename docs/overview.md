## Overview

Cognex is a lightweight, persistent key–value and search database written in C++.

It is designed as an educational storage engine that demonstrates:

- Log‑structured storage.
- Write‑Ahead Logging (WAL) and crash recovery.
- Snapshot‑based persistence.
- Inverted index–backed full‑text search with BM25‑style ranking.

---

## What Cognex Provides

- **Key–value database**
  - String keys and values.
  - `PUT`, `GET`, and `DEL` operations.
- **Durability**
  - WAL with CRC32 checksums.
  - Append‑only value log with checksummed records.
  - Periodic snapshots of the key index.
- **Search**
  - Tokenization of document values.
  - Inverted index (`token → posting list`).
  - BM25‑inspired ranking and simple numeric filters.
- **CLI / REPL**
  - Interactive prompt for issuing commands (`PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `HELP`, `EXIT`).
- **Benchmarks**
  - Built‑in benchmark harness (`bench/bench.cpp`) for PUT / GET / QUERY workloads.

---

## Architecture at a Glance

Core components:

- **`Cognex`**
  - Implements the `DB` and `Persistence` interfaces.
  - Orchestrates storage, indexing, and query engines.
- **`StorageEngine`**
  - Manages WAL, snapshot, and value‑log files.
  - Provides `recover`, `append_wal_record`, `append_to_value_log_`, `read_from_log_`, and `snapshot`.
- **`IndexEngine`**
  - Tokenizes values and maintains:
    - `entries_` – stored entries with tokens.
    - `postings_` – inverted index for search.
- **`QueryEngine`**
  - Executes `Query` objects using postings and document statistics.
  - Applies filters and ranking to produce `QueryResult` lists.

For a detailed breakdown, see `architecture.md`, `storage-engine.md`, `indexing.md`, and `query-engine.md`.

---

## Documentation Map

The `/docs` directory is organized as follows:

- **High‑level**
  - `overview.md` – this document.
  - `architecture.md` – component‑level architecture and flows.
- **Engine internals**
  - `storage-engine.md` – value log, WAL/snapshot integration, recovery.
  - `indexing.md` – tokenization, entries, postings, inverted index.
  - `query-engine.md` – query model, ranking, filters, CLI integration.
  - `wal.md` – WAL layout and replay.
  - `snapshot.md` – snapshot behavior and trade‑offs.
- **Behavior and usage**
  - `guide.md` – getting started and CLI usage examples.
  - `cli.md` – REPL, command parsing, and command implementations.
- **Performance and versions**
  - `benchmarks.md` – benchmark harness and results.
  - `v3.md` – historical notes (see file for details).
- **Reference**
  - `api.md` – public interfaces and CLI commands.
  - `classes.md` – main classes and responsibilities.
  - `types.md` – core data structures.
- **Development**
  - `development.md` – building, running, and contributing to the engine.
  - `server.md` – placeholder; no server implementation exists yet (**TODO**).

