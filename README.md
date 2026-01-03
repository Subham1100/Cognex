# Cognex

Cognex is a single-node key–value database built incrementally
to understand storage engine fundamentals.

The project evolves through well-defined versions, each adding
one core database capability while preserving correctness.

---

## Current State

### v0 — In-Memory KV Store
- In-memory key–value storage
- Single-threaded
- No persistence

### v1 — Write-Ahead Logging (Current)
- Persistent storage using WAL
- Crash recovery via log replay
- Single-node, single-threaded
- Deterministic durability semantics

---

## Guarantees (v1)

- A write is considered **committed only after WAL fsync**
- Crashes after fsync do **not lose data**
- Crashes before fsync **may lose recent writes**
- Recovery deterministically rebuilds state by replaying WAL

---

## Non-Goals (v1)

- No concurrency
- No snapshots
- No WAL truncation
- No performance optimizations

These are intentionally deferred to later versions.

---

## Usage

### Prerequisites
- C++17 compatible compiler
- CMake >= 3.10
- Git

---

### Build

```bash
git clone https://github.com/Subham1100/Cognex.git
cd Cognex/versions/cognex_v1

mkdir build
mkdir -p data
cd build

cmake ..
make
