# Snapshotting

Snapshots store the full database state.

---

## Purpose

- Faster crash recovery
- Reduce WAL replay time
- Bound WAL growth

---

## Behavior

- Manually via the SNAPSHOT command
- Automatically after a configurable write threshold

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

## Tradeoffs

| Benefit            | Cost                         |
| ------------------ | ---------------------------- |
| Faster recovery    | Snapshot IO overhead         |
| Smaller WAL replay | Periodic write amplification |
