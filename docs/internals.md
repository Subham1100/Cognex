# Internals

This section briefly explains core internal mechanisms of Cognex.

---

## Durability Model

Cognex uses:

- **Write-Ahead Log (WAL)** – Records mutations
- **Snapshots** – Persist full state

---

## Write Path

1. Command received
2. Mutation written to WAL
3. Change applied to storage

---

## Recovery Path

1. Load latest snapshot (if present)
2. Replay WAL entries
3. Restore consistent state

---

## Design Focus

- Simplicity
- Crash safety
- Deterministic recovery