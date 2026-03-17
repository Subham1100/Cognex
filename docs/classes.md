# Classes

Overview of core classes in Cognex.

---

## **Cognex**

Primary database implementation and façade over the storage, indexing, and query engines.

**Inheritance**

- `DB`
- `Persistence`

---

## **Constructor**

### `Cognex(WalPath wal_path, SnapshotPath snapshot_path, ValueLogPath value_log_path)`

Initializes the database with persistence paths. Internally, these are passed to `StorageEngine`.

**Parameters**

- `wal_path` – WAL file location
- `snapshot_path` – Snapshot file location
- `value_log_path` – Append-only ValueLog file

---

## **Public Methods**

### `put(Key key, Value value)`

Stores or updates a key–value pair.

- Appends a textual `PUT` record to the WAL via `StorageEngine`.
- Appends the value (with header and checksum) to the value log.
- Updates the in-memory offset-based index (`Key → IndexEntry`).
- Tokenizes the value and updates the inverted index (`postings_`) via `IndexEngine`.
- Overwrites existing values if the key is already present.

---

### `get(const Key& key) -> std::optional<Value>`

Retrieves value associated with a key.

- Resolves the value-log offset and size via the in-memory index.
- Reads and verifies the record using `StorageEngine::read_from_log_`.

---

### `del(const Key& key) -> bool`

Deletes a key from the index.

- Records a `DEL` mutation in the WAL.
- Removes the index entry if present and returns `true`; otherwise returns `false`.

---
### `query(const Query& query) -> std::vector<QueryResult>`

Executes a full-text query over stored values.

- Delegates to `QueryEngine::execute`.
- Uses the inverted index and BM25-based scoring.
- Applies filters and sorting as specified in the `Query`.

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

- `std::unordered_map<Key, IndexEntry> index_` – key → `{offset, valueSize}` in the value log.
- `std::vector<Entry> entries_` – stored entry metadata, including tokens.
- `std::unordered_map<std::string, std::vector<Posting>, TransparentHash, TransparentEqual> postings_` – inverted index.
- `size_t snapshotWriteOps_`, `size_t snapshotEveryNWriteOps_` – write counter and threshold for automatic snapshots at the engine level.
- `size_t totalTokens_` – total number of tokens across all entries (used by the query engine).
- `QueryEngine queryEngine_` – query execution and ranking.
- `IndexEngine indexEngine_` – tokenization and index maintenance.
- `StorageEngine storageEngine_` – WAL, snapshot, and value-log management.

---

## **Responsibilities**

- Implements DB interface
- Coordinates persistence
- Ensures durability & recovery
- Maintains inverted index
- Exposes search over values via `QueryEngine`
