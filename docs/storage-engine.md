## Storage Engine

This document describes the `StorageEngine` implementation and how it interacts with `Cognex`.

All details are derived from the current code in `engine/include/storage/` and `engine/src/storage/`.

---

## Responsibilities

`StorageEngine` is responsible for:

- Managing on‑disk files:
  - `wal.log` (Write‑Ahead Log)
  - `snapshot.dat` (snapshot of the key index)
  - `value.log` (append‑only value log)
- Providing persistence primitives for `Cognex`:
  - `recover`
  - `append_wal_record`
  - `append_to_value_log_`
  - `read_from_log_`
  - `snapshot`

`Cognex` uses these primitives to implement `put`, `get`, `del`, `recover`, and `snapshot`.

---

## Data Layout

### Value Log

The value log stores values sequentially, each as:

```text
[RecordHeader { keySize, valueSize }][value bytes][checksum]
```

Where:

- `RecordHeader`:
  - `uint32_t keySize`
  - `uint32_t valueSize`
- `value bytes`:
  - Raw bytes of the `Value` string.
- `checksum`:
  - CRC32 over the header and value:
    - Computed with `crc32_buf` and `crc32_extend`.

The in‑memory index in `Cognex` maps `Key` → `IndexEntry{offset, valueSize}` into this file.

### WAL

The WAL logs logical operations:

- Text records such as `"PUT key value"` and `"DEL key"`.
- Written as:

```text
[uint32_t len][len bytes of record][uint32_t checksum]
```

See `wal.md` for full details.

### Snapshot

Snapshots persist the in‑memory key index:

- `write_snapshot_atomic` and `load_snapshot` operate on:

```cpp
std::unordered_map<Key, IndexEntry>
```

Only the index is snapshotted; values remain in the value log and are accessed by offset.

---

## Key Methods

### Constructor

```cpp
StorageEngine::StorageEngine(WalPath wal,
                             SnapshotPath snapshot,
                             ValueLogPath valueLog);
```

Stores path wrappers and does not open any files immediately.

---

### `recover`

```cpp
void StorageEngine::recover(std::unordered_map<Key, IndexEntry>& index_,
                            std::function<void(std::string_view)> apply_record_);
```

Recovery steps:

1. Ensure the value log is open (`open_value_log_if_needed_`).
2. Load snapshot:
   - `load_snapshot(snapshot_path_, index_)`.
3. Replay WAL:
   - `wal_replay(wal_path_, apply_record_)`.

The `apply_record_` callback is provided by `Cognex` and interprets `"PUT"` and `"DEL"` records to rebuild in‑memory state.

After this, `Cognex::recover` additionally rebuilds the search index from the restored key index by reading values via `read_from_log_`.

---

### `append_to_value_log_`

```cpp
uint64_t StorageEngine::append_to_value_log_(uint32_t keySize,
                                             std::string_view value);
```

Implementation:

- Lazily opens the value log (if not already open).
- Seeks to end and records the current offset.
- Constructs a `RecordHeader` with `keySize` and `value.size()`.
- Computes CRC32 checksum over header and value.
- Writes:
  - Header
  - Value bytes
  - Checksum
- Increments `valueLogWrites_` and occasionally `fsync`s when it reaches `valueLogFsyncEveryNWrites_`.
- Returns the starting offset of the record.

`Cognex::put` uses this offset to create an `IndexEntry`.

---

### `read_from_log_`

```cpp
std::optional<Value> StorageEngine::read_from_log_(uint64_t offset,
                                                   uint32_t keySize) const;
```

Implementation:

- Requires `valueLogFd_` to be open (opened by `open_value_log_if_needed_` during recovery or first append).
- Uses `lseek` and `pread_all` to:
  - Read `RecordHeader` at `offset`.
  - Validate `header.keySize == keySize`.
  - Read `header.valueSize` bytes for the value.
  - Read the stored checksum.
  - Recompute the checksum and compare.
- On success, returns `Value{std::move(value)}`.
- On mismatch or IO error, throws an exception.

This is used by `Cognex::get` and by `Cognex::recover` when rebuilding search structures.

---

### `append_wal_record`

```cpp
void StorageEngine::append_wal_record(const std::string& record);
```

- Delegates to:

```cpp
append_and_fsync(wal_path_, record, walWrites_, walFsyncEveryNWrites_);
```

- `walWrites_` and `walFsyncEveryNWrites_` control fsync batching (see `wal.md`).

`Cognex::put` and `Cognex::del` use this for durability.

---

### `snapshot`

```cpp
void StorageEngine::snapshot(const std::unordered_map<Key, IndexEntry>& index_);
```

- Writes the current key index to disk:
  - `write_snapshot_atomic(snapshot_path_, index_);`
- Truncates the WAL:
  - `wal_truncate(wal_path_);`

`Cognex::snapshot` calls this method, and the CLI can trigger it via the `SNAPSHOT` command.

---

## Interaction with `Cognex`

High‑level flows:

- **PUT**
  - `append_wal_record("PUT key value")`
  - `append_to_value_log_(keySize, value)`
  - Update `index_` and search structures.
  - Increment snapshot counters.

- **GET**
  - Lookup `IndexEntry` in `index_`.
  - `read_from_log_` to fetch the value.

- **DEL**
  - `append_wal_record("DEL key")`
  - Remove key from `index_`.

- **recover**
  - `StorageEngine::recover` restores `index_` from snapshot + WAL.
  - `Cognex::recover` reads values back from the value log and rebuilds the inverted index.

**TODO:** Value‑log compaction / garbage collection is not implemented; deleted or superseded values remain in `value.log`.

