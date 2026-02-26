# Internals

This section briefly explains core internal mechanisms of Cognex.

---

## Durability Model

Cognex uses:

- **Write-Ahead Log (WAL)** – Records mutations
- **ValueLog** – Append-only storage for values
- **Snapshots** – Persist full state

---

## Write Path

~~~
Command received
   ↓
Mutation appended to WAL
   ↓
WAL fsync (batched)
   ↓
Value appended to ValueLog
   ↓
Index updated (key → offset)
   ↓
Tokenizer / inverted index updated
~~~

---

## Read Path

~~~
GET request
   ↓
Index lookup (key → offset)
   ↓
pread(ValueLog, offset)
~~~

Cognex uses offset-based reads to avoid loading full ValueLog data into memory.

---

## Recovery Path

~~~
Load latest snapshot (if present)
   ↓
Replay WAL entries
   ↓
Rebuild index/state
   ↓
Ready
~~~

---

## fsync Strategy

To balance durability and performance:

- WAL fsync is batched
- ValueLog writes are append-only
- Snapshots reduce WAL replay overhead