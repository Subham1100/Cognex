# Storage Engine (High Level)

This document provides a brief high-level view of the storage engine.  
For a deeper, implementation-level description see `storage-engine.md`.

---

## Data Model

- Unique string **keys**.
- String **values**.
- Values are stored on disk in an append‑only **value log**.
- An in‑memory index maps `Key → IndexEntry{offset, valueSize}` into the value log.

---

## Supported Operations

- **PUT** – Insert or overwrite a value for a key.
- **GET** – Retrieve the value for a key.
- **DEL** – Remove a key (and its index entry; search entries may be tombstoned and compacted in memory).

---

## Behavior

- `PUT`:
  - Always overwrites the previous value for the key.
  - Appends to WAL and value log before updating the in‑memory index.
- `GET`:
  - Uses the in‑memory index to locate the value on disk and reads it by offset.
- `DEL`:
  - Records a delete in the WAL and removes the key from the in‑memory key index.
  - In-memory search structures can be compacted via `Cognex::compact()` (also triggered automatically after a number of deletes, and via the `COMPACT` CLI command).
  - Space in the on-disk value log is still not reclaimed; **value log GC remains TODO.**

---

## Persistence

Data durability is achieved via:

- **WAL** – Records `PUT` and `DEL` operations before they are applied.
- **Snapshots** – Periodically persist the key index to disk so recovery can replay a shorter WAL.

See `wal.md`, `snapshot.md`, and `storage-engine.md` for full details.