# Snapshotting

Snapshots store the full database state.

---

## Purpose

- Faster recovery
- Limit WAL replay

---

## Behavior

- Created via `SNAPSHOT` command
- Used as recovery baseline

---

## Recovery

Snapshot + WAL → Restored state