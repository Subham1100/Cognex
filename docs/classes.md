# Classes

Overview of core classes in Cognex.

---

## **Cognex**

Primary database implementation.

**Inheritance**

- `DB`
- `Persistence`

---

## **Constructor**

### `Cognex(WalPath wal_path, SnapshotPath snapshot_path, ValueLogPath value_log_path)`

Initializes the database with persistence paths.

**Parameters**

- `wal_path` – WAL file location
- `snapshot_path` – Snapshot file location
- `value_log_path` – Append-only ValueLog file

---

## **Public Methods**

### `put(Key key, Value value)`

Stores or updates a key–value pair.

- Appends mutation to WAL
- Appends value to ValueLog
- Updates offset-based index
- Updates inverted index
- Overwrites existing values

---

### `get(const Key& key) -> std::optional<Value>`

Retrieves value associated with a key.

- Resolves offset via index
- Reads value using pread()

---

### `del(const Key& key) -> bool`

Deletes a key from the index.

- Mutation recorded in WAL
- Index entry removed

---
### `query(std::string_view token)`

Returns entries containing the token.

- Uses inverted index lookup

---

### `recover()`

Restores database state using:

- Snapshot (if present)
- WAL replay
- Rebuilds index/state

---

### `snapshot()`

Persists current database state.

---

## **Private Members**

- `wal_path_` – WAL file path
- `snapshot_path_` – Snapshot file path
- `valueLogFd_` – ValueLog file descriptor
- `valuelog_path_` – ValueLog file path
- `index_` – Key → {offset, valueSize}
- `entries_` – Stored entry metadata
- `tokenIndex_` – Token → posting list
- `postings_` – Posting list storage

---

## **Responsibilities**

- Implements DB interface
- Coordinates persistence
- Ensures durability & recovery
- Maintains inverted index
