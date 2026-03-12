# Cognex

Cognex is a lightweight persistent key–value database written in C++.

It is designed to explore database internals — durability, crash recovery, and storage engine mechanics — through a minimal yet practical implementation.


---

 [Documentation](https://subham1100.github.io/Cognex/)

## What Cognex Provides

- Write-Ahead Logging (WAL) for durability and crash safety
- Append-only ValueLog storage for efficient sequential writes
- Offset-based indexing for fast value retrieval
- Batched fsync strategy for performance tuning
- Periodic snapshots for reduced recovery time
- Crash-safe recovery via snapshot + WAL replay
- Interactive CLI (REPL) for direct database interaction
- Token-based QUERY support powered by an inverted index
- Automatic storage directory (~/.cognex)

---
---

## PUT Benchmark

**Workload:**  
Inserted **1,000,000 key–value pairs**

| Metric | Value |
|--------|--------|
| Operations | 1,000,000 |
| Total Time | 43.029 sec |
| Throughput | **23240.1 ops/sec** |

---

## GET Benchmark

**Workload:**  
Performed **1,000,000 random key lookups**

| Metric | Value |
|--------|--------|
| Operations | 1,000,000 |
| Total Time | 21.527 sec |
| Throughput | **46453.3 ops/sec** |

---

##  Mixed Benchmark (50% PUT / 50% GET)

**Workload:**  
Alternating PUT and GET operations

| Metric | Value |
|--------|--------|
| Operations | 1,000,000 |
| Total Time | 19.174 sec |
| Throughput | **52,154 ops/sec** |

---

##  Observations

- **PUT performance** is significantly slower due to WAL writes and persistence overhead.
- **GET operations** are extremely fast since values are currently served from memory.
- Mixed workload reflects realistic usage patterns.

---

## Installation

### Option 1: One-line install (recommended)

```bash
curl -fsSL https://raw.githubusercontent.com/Subham1100/Cognex/main/install.sh | bash
```
Run with : ./bin/Cognex

Option 2: Install from source (manual)
```
git clone https://github.com/Subham1100/Cognex.git
cd Cognex
chmod +x install.sh
./install.sh
```
Run with : ./bin/Cognex

## Example Usage

Inside the Cognex prompt:
```
> PUT ("name") ("Alice is good") ---> entry 0
> PUT ("n") ("Bob is good") -----> entry 1
[Sucess]
> GET ("name")
Alice is good
> QUERY ("good")
> 0
> 1
> DEL name
[Sucess]
> GET name
[NIL]

```

## This directory contains:

- Write-ahead log files
- Snapshot files
- etadata required for recovery

## On startup, Cognex automatically:

- Loads the latest snapshot (if present)
- Replays the WAL
- Restores the database to a consistent state

## Crash Safety & Recovery

- Every write is first recorded in the WAL
- Periodic snapshots reduce recovery time
- On crash or restart, Cognex guarantees no committed data is lost

## Build Requirements

To build Cognex from source, you need:

- CMake ≥ 3.16
- A C++17-compatible compiler (gcc / clang)

## License
MIT License
