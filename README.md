# Cognex

Cognex is a single-node, persistent key–value database built incrementally to understand real database internals such as durability, crash recovery, and storage invariants.

The project is developed **version by version**, with each version freezing a clear set of guarantees before moving forward.  
Cognex prioritizes **correctness and clarity over performance**.

---

## Project Goals

- Learn how real databases are built internally
- Implement correctness before optimization
- Evolve features incrementally with explicit guarantees
- Keep the system simple, inspectable, and auditable

---

## Version Overview

| Version | Description |
|------|------------|
| v0 | In-memory key–value store |
| v1 | Write-Ahead Log (WAL) with crash recovery |
| v2 | Snapshots (checkpointing) and WAL truncation |
| v3 (planned) | Concurrent reads and writes |

---

## Cognex v2 — Snapshots & WAL Truncation

Cognex v2 extends the durability guarantees of v1 by introducing **snapshots (checkpoints)**.  
This makes recovery time and disk usage **bounded**, turning Cognex into a practical single-node storage engine.

---

## Why v2 Exists

In v1, all writes were persisted using a Write-Ahead Log (WAL).  
While correct, this approach had inherent limitations:

- WAL size grew indefinitely
- Startup time increased linearly with WAL length

These are expected constraints of a WAL-only system.

v2 solves this by introducing snapshotting.

---

## Key Features in v2

### Snapshots (Checkpointing)

Cognex periodically writes the full in-memory state to disk as a snapshot.

- Represents a fully materialized database state
- Independent of WAL history
- Stored in a simple, human-readable format

