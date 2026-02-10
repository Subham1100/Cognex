# Cognex

Cognex is a **simple, persistent key–value database** written in C++.

It is built to explore **database internals** such as durability, crash recovery, and storage engines, using:

- Write-Ahead Logging (WAL)
- Periodic snapshots
- A built-in interactive CLI (REPL)

Cognex is lightweight, crash-safe, and designed for learning, experimentation, and systems-level understanding.

---

## Features

- Persistent key–value storage
- Write-Ahead Log (WAL) for durability
- Snapshot-based fast recovery
- Interactive command-line interface (REPL)
- Automatic data storage in `~/.cognex`
- No runtime dependencies after installation

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
> PUT name Alice
[Sucess]
> GET name
Alice
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
