# Cognex

Cognex is a **simple, persistent key–value database** written in C++.

It is built to explore **database internals** such as durability, crash recovery, and storage engines, using:

- Write-Ahead Logging (WAL)
- Periodic snapshots
- A built-in interactive CLI (REPL)

Cognex is lightweight, crash-safe, and designed for learning, experimentation, and systems-level understanding.

---

 [Documentation](https://subham1100.github.io/Cognex/)

## Features

Persistent key–value storage
Write-Ahead Log (WAL) for durability
Snapshot-based fast recovery
Interactive command-line interface (REPL)
Token-based QUERY support (inverted index)
Automatic data storage in ~/.cognex
No runtime dependencies after installation

---
---

## PUT Benchmark

**Workload:**  
Inserted **1,000,000 key–value pairs**

| Metric | Value |
|--------|--------|
| Operations | 1,000,000 |
| Total Time | 39.501 sec |
| Throughput | **25,315.8 ops/sec** |

---

## GET Benchmark

**Workload:**  
Performed **1,000,000 random key lookups**

| Metric | Value |
|--------|--------|
| Operations | 1,000,000 |
| Total Time | 0.61 sec |
| Throughput | **1,639,340 ops/sec** |

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
> PUT ("name") ("Bob is good") -----> entry 1
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
