# API Reference

This document describes the public interfaces exposed by Cognex, as implemented in the current codebase.

---

## Database Commands (CLI)

Commands are parsed by the CLI (`parser`, `CommandRegistry`, `ICommand` implementations) and executed against a `Cognex` instance:

- **`PUT "<key>" "<value>"`**
  - Stores or updates a key–value pair.
  - Creates the key if missing.
  - Overwrites the existing value if the key already exists.

- **`GET "<key>"`**
  - Retrieves the value associated with a key.
  - On success: prints the stored value.
  - On missing key: prints `[NIL]`.

- **`DEL "<key>"`**
  - Deletes a key from storage.
  - No error is thrown if the key does not exist.

- **`QUERY "<terms>" [filters…]`**
  - Performs a full‑text search over values using the inverted index.
  - The first quoted argument is split on whitespace into one or more search terms.
  - Optional additional arguments are filters, for example:
    - `relevance >= 2`
    - `similarity > 50`
    - `top = 20`
    - `sortby similarity`
  - Matching entries are printed with:
    - `entryId`
    - `key`
    - `relevance`
    - `similarity`

- **`SNAPSHOT`**
  - Triggers `Cognex::snapshot()` to persist the current key index to disk and truncate the WAL.

- **`COMPACT`**
  - Calls `Cognex::compact()` to compact in‑memory search structures (drop tombstoned entries, rebuild postings). Does not reclaim space in `value.log`.

- **`HELP`**
  - Prints available commands.

- **`EXIT`**
  - Exits the REPL loop.

See `cli/` sources and `docs/cli.md` for details.

---

## DB and Persistence Interfaces

Defined in `core/db.h`:

```cpp
struct DB {
    virtual ~DB() = default;

    virtual void put(Key key, Value value) = 0;
    virtual std::optional<Value> get(const Key& key) const = 0;
    virtual bool del(const Key& key) = 0;
};

struct Persistence {
    virtual ~Persistence() = default;

    virtual void recover() = 0;
    virtual void snapshot() = 0;
};
```

`class Cognex` implements both `DB` and `Persistence`.

Additional engine method (not part of `DB` / `Persistence`):

- **`void compact()`**
  - Rebuilds `entries_`, `postings_`, `keyToEntry_`, and `totalTokens_` after deletes. See `docs/cli.md` and `docs/storage-engine.md`.

---

## WAL API

Implemented in `storage/wal.h` and `storage/wal.cpp`:

- **`append_and_fsync`**

  Appends a record to the WAL and periodically fsyncs the file:

  ```cpp
  void append_and_fsync(const WalPath& path,
                        const std::string& record,
                        size_t& walWrites_,
                        size_t& walFsyncEveryNWrites_);
  ```

  - Writes:
    - `uint32_t len`
    - `len` bytes of `record`
    - `uint32_t checksum` (CRC32 of `record`)
  - Increments `walWrites_` and calls `fsync` when it reaches `walFsyncEveryNWrites_`.

- **`wal_replay`**

  Template function that replays WAL records sequentially:

  ```cpp
  template<typename ApplyFn>
  bool wal_replay(const WalPath& path, ApplyFn apply);
  ```

  - Reads:
    - `len`
    - `record` (length `len`)
    - `checksum`
  - Validates the checksum with `crc32_str(record)`.
  - Invokes `apply(std::string_view(record))` for each valid record.

- **`wal_truncate`**

  Truncates the WAL file and fsyncs it:

  ```cpp
  void wal_truncate(const WalPath& path);
  ```

  Used after a successful snapshot to reset the WAL.

---

## Snapshot API

Implemented in `storage/snapshot.h`:

- **`write_snapshot_atomic`**

  Writes a snapshot of the in‑memory index using atomic replacement:

  ```cpp
  void write_snapshot_atomic(
      const SnapshotPath& path,
      const std::unordered_map<Key, IndexEntry>& index_);
  ```

  - Persists the mapping `Key → IndexEntry{offset, valueSize}`.
  - **Note:** previous versions wrote `Key → Value`; current implementation writes the index only.

- **`load_snapshot`**

  Loads a snapshot into the in‑memory index:

  ```cpp
  void load_snapshot(
      const SnapshotPath& path,
      std::unordered_map<Key, IndexEntry>& index_);
  ```

  - Restores the key index used for value‑log lookups.

Snapshots are consumed by `StorageEngine::recover` and coordinated by `Cognex::recover`.

---

## Query Engine API (High‑Level)

Defined in `query/query_engine.h`:

```cpp
class QueryEngine {
public:
    std::vector<QueryResult> execute(
        const Query& query,
        const std::unordered_map<std::string,
                                 std::vector<Posting>,
                                 TransparentHash,
                                 TransparentEqual>& postings_,
        size_t totalDocs,
        size_t totalTokens_,
        const std::vector<Entry>& entries_) const;
};
```

The main entrypoint is `execute`, which:

- Generates candidates from the inverted index (`postings_`).
- Applies filters from `Query::filters`.
- Ranks and selects top‑K results according to `Query::sortBy` and `Query::topK`.

See `docs/query-engine.md` for a detailed description of the query model and ranking behavior.