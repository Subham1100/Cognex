# Write-Ahead Log (WAL)

The WAL ensures durability and crash recovery.

---

## Principle

Log first → Apply later

---

## Logged Operations

Cognex records logical mutations:

- PUT – Insert / update key
- DEL – Remove key


---
## Record Structure

Each WAL entry contains:

- Record length
- Serialized command

Checksum (CRC32)
---
## fsync Strategy

~~~
size_t walWrites_;
size_t walFsyncEveryNWrites_;
~~~
- WAL writes are accumulated
- fsync() is batched
- Reduces disk sync overhead

---

## Recovery Process

On restart:

~~~
Load Snapshot (if present)
   ↓
Replay WAL entries
   ↓
Restore consistent state
~~~

Invalid / partial WAL records are safely ignored using checksum validation.

---

## Interaction with Snapshots

- Snapshot provides recovery baseline
- WAL replays mutations after snapshot
- WAL may be truncated after snapshot