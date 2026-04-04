# Getting Started

## Installation

### Option 1: One-line install (recommended)

```bash
curl -fsSL https://raw.githubusercontent.com/Subham1100/Cognex/main/install.sh | bash
```
Run with : ```./bin/Cognex```

Option 2: Install from source (manual)
```
git clone https://github.com/Subham1100/Cognex.git
cd Cognex
chmod +x install.sh
./install.sh
```
Run with : ```./bin/Cognex```

## Example Session

Arguments use double quotes (see `cli/src/parser.cpp`):

~~~
cognex> PUT "name" "Alice is good"
[Success]

cognex> GET "name"
Alice is good

cognex> QUERY "good"
Found entries:
  0 key=name relevance=... similarity=...

cognex> DEL "name"
[Success]

cognex> GET "name"
[NIL]
~~~

## Commands

### **PUT `"key"` `"value"`**

Stores a value under the specified key.

- If the key does **not exist** → creates a new entry
- If the key **exists** → overwrites the value

**Example**

PUT "name" "Alice"
PUT "age" "20"


---

### **GET `"key"`**

Retrieves the value associated with a key.

- Returns the stored value if key exists
- Returns an error / null if key does not exist

**Example**

GET "name"

**Output**

Alice


---

### **DEL `"key"`**

Deletes a key and its value from storage.

- No-op if key does not exist

**Example**

DEL "age"

---
### **QUERY `"terms"` [filters…]**

Full‑text search over stored values using the inverted index. The first argument is split on whitespace into terms; optional extra quoted arguments set `top`, `relevance` / `similarity` filters, and `sortby`. See `docs/query-engine.md` and `docs/cli.md`.

**Example**

QUERY "alice"
QUERY "database engine" "relevance >= 2" "top = 20" "sortby similarity"

---

### **SNAPSHOT**

Triggers creation of a snapshot of the current database state.

Snapshots allow:

- Faster recovery
- Reduced WAL replay
- Durable persistence

**Example**

SNAPSHOT

---

### **COMPACT**

Runs `Cognex::compact()` to drop tombstoned search entries and rebuild postings in memory. Does not shrink the on-disk value log.

**Example**

COMPACT

---

### **HELP**

Displays available commands.

---

### **EXIT**

Gracefully shuts down Cognex.

---

## Storage behavior

### **Key–Value Model**

- Keys are unique
- Values are stored as raw strings (current implementation)

---

### **Overwrite Semantics**

`PUT` always replaces existing values.

PUT "x" "10"
PUT "x" "20"
GET "x" → 20


---

### **Persistence**

Depending on your current implementation, Cognex may persist data using:

- Write-Ahead Log (WAL)
- Snapshot files

See:

- [WAL](wal.md)
- [Snapshotting](snapshot.md)

---

## Current limitations

- Values are treated as strings
- No secondary indexes
- No range queries
- No TTL / expiration
- No transactions 

---

## Future Enhancements (Planned)

- Typed values
- On-disk value log compaction / garbage collection (space reclamation in `value.log`)
- Concurrency control
- Transactions / MVCC
- Compression
