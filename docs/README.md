# Cognex Documentation

This directory contains the structured documentation for Cognex.

For a high‑level introduction to the project, see:

- `overview.md` – conceptual overview and feature list.
- Root‑level `README.md` – project description, quickstart, and benchmarks.

---

## Contents

- **Overview and Architecture**
  - `overview.md` – what Cognex is and what it provides.
  - `architecture.md` – core components, flows, and durability model.
  - `internals.md` – write, read, and recovery paths at a glance.

- **Engine Internals**
  - `storage-engine.md` – value log, WAL/snapshot integration, recovery.
  - `indexing.md` – tokenization and inverted index structures.
  - `query-engine.md` – query model, ranking, and filters.
  - `storage.md` – high‑level storage behavior.
  - `wal.md` – WAL format and replay.
  - `snapshot.md` – snapshot behavior and trade‑offs.

- **Usage and Interface**
  - `guide.md` – getting started and example CLI sessions.
  - `cli.md` – REPL, parsing, and command semantics.
  - `api.md` – DB/Persistence interfaces, WAL/snapshot APIs, query engine entrypoints, and `Cognex::compact()`.

- **Performance and Versions**
  - `benchmarks.md` – benchmark harness and current results.
  - `v3.md` – historical notes about earlier benchmark numbers.

- **Reference Types and Classes**
  - `classes.md` – main classes and their responsibilities.
  - `types.md` – core data structures used across the engine.

- **Development**
  - `development.md` – building, running, and contribution guidelines.
  - `server.md` – notes on the (currently absent) server component and future TODOs.

All documents are kept in sync with the current implementation; if you change the code, please update the corresponding files here.
