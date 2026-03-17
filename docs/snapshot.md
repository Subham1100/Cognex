# Snapshotting

Snapshots store the full database state.

---

## Purpose

- Faster crash recovery
- Reduce WAL replay time
- Bound WAL growth

---

## Behavior

Snapshots are created:

- Manually via the `SNAPSHOT` CLI command (calls `Cognex::snapshot()`).
- Automatically:
  - In the CLI REPL when the number of write commands reaches a small threshold (see `SNAPSHOT_THRESHOLD` in `repl.cpp`).
  - In the engine when `snapshotWriteOps_` reaches `snapshotEveryNWriteOps_` inside `Cognex::put` (current default is large and acts as a safety net).

~~~
size_t snapshotWriteOps_;
size_t snapshotEveryNWriteOps_;
~~~

---

## Recovery

~~~
Load Snapshot (if present)
   ↓
Replay WAL entries
   ↓
Restore consistent state
~~~

Implementation details:

- `write_snapshot_atomic` persists the in‑memory index `std::unordered_map<Key, IndexEntry>` to disk, using atomic replacement semantics.
- `load_snapshot` restores this index from disk.
- After snapshotting, `wal_truncate` is used to clear the WAL so that future recoveries replay only new operations.

## Tradeoffs

| Benefit            | Cost                         |
| ------------------ | ---------------------------- |
| Faster recovery    | Snapshot IO overhead         |
| Smaller WAL replay | Periodic write amplification |
