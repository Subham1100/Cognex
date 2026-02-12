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

### `Cognex(WalPath wal_path, SnapshotPath snapshot_path)`

Initializes the database with persistence paths.

**Parameters**

- `wal_path` – WAL file location
- `snapshot_path` – Snapshot file location

---

## **Public Methods**

### `put(Key key, Value value)`

Stores or updates a key–value pair.

- Overwrites existing values

---

### `get(const Key& key) -> std::optional<Value>`

Retrieves value for a key.

- Returns value if present
- Returns empty optional if missing

---

### `del(const Key& key) -> bool`

Deletes a key.

- Returns true if deleted
- Returns false if key not found

---

### `recover()`

Restores database state using:

- Snapshot (if present)
- WAL replay

---

### `snapshot()`

Persists current database state.

---

## **Private Members**

- `wal_path_` – WAL path
- `snapshot_path_` – Snapshot path

---

## **Responsibilities**

- Implements DB interface
- Coordinates persistence
- Ensures durability & recovery