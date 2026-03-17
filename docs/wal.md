# Write‑Ahead Log (WAL)

The WAL ensures durability and crash recovery by logging mutations before they are applied to the in‑memory index and value log.

---

## Principle

**Log first → Apply later**

All `PUT` and `DEL` operations are:

1. Serialized as text records (e.g. `"PUT <key> <value>"`, `"DEL <key>"`).
2. Appended to the WAL file.
3. Periodically flushed to disk (`fsync`) in batches.

---

## Logged Operations

Cognex records the following logical mutations:

- `PUT` – insert or update a key.
- `DEL` – remove a key.

The exact text format is constructed in `Cognex::put` and `Cognex::del`.

---

## Record Structure

Each WAL entry is written as:

```text
[uint32_t len][len bytes of record][uint32_t checksum]
```

Where:

- `len` – size of the textual `record` in bytes.
- `record` – UTF‑8 string such as `"PUT key value"`.
- `checksum` – CRC32 of the `record` (computed via `crc32_str`).

This layout is implemented in `append_and_fsync` and consumed by `wal_replay`.

---

## fsync Strategy

Two counters control how often the WAL is synced:

```cpp
size_t walWrites_;
size_t walFsyncEveryNWrites_;
```

- After each append, `walWrites_` is incremented.
- When `walWrites_ >= walFsyncEveryNWrites_`, `fsync(fd)` is called and `walWrites_` is reset to zero.
- This batching reduces sync overhead at the cost of potentially losing up to `walFsyncEveryNWrites_ - 1` most recent WAL entries in a crash.

The exact thresholds are configured in `StorageEngine`.

---

## Recovery Process

On restart, recovery runs as:

```text
Load snapshot (if present)
   ↓
wal_replay(path, apply_record)
   ↓
Rebuild search index from recovered key index
   ↓
Ready
```

- `StorageEngine::recover` calls `load_snapshot` to populate the in‑memory key index.
- Then `wal_replay` reads and validates each record:
  - If the CRC32 checksum matches, it calls the provided `apply_record` callback with the record contents.
- The `apply_record_` implementation in `Cognex` re‑applies `PUT` and `DEL` mutations.

Invalid or partial WAL records (e.g. due to a crash mid‑write) are detected via checksum mismatch or short reads, and replay stops rather than applying corrupt data.

---

## Interaction with Snapshots

- Snapshots provide a **baseline** state for recovery.
- WAL replays only the operations that occurred **after** the snapshot.
- After writing a snapshot, `StorageEngine::snapshot` calls `wal_truncate` to clear the WAL and start a new log from an empty state.

This keeps WAL size bounded and reduces recovery time.