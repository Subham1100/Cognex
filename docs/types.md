# Types

Core data structures used in Cognex, as defined in `core/types.h`.

---

## **WalPath**

Represents the filesystem location of the Write‑Ahead Log.

**Fields**

- `std::string value`

**Purpose**

- Encapsulates the WAL file path.

---

## **SnapshotPath**

Represents the filesystem location of a snapshot.

**Fields**

- `std::string value`

**Purpose**

- Encapsulates the snapshot file path.

---

## **ValueLogPath**

Represents the filesystem location of the value log.

**Fields**

- `std::string value`

**Purpose**

- Encapsulates the value‑log file path used by `StorageEngine`.

---

## **Key**

Represents a unique identifier for stored data.

**Fields**

- `std::string value`

**Behavior**

- Equality comparison (`operator==`).
- Hashable via `std::hash<Key>` specialization.

---

## **Value**

Represents stored data.

**Fields**

- `std::string value`

---

## **Entry**

Represents a stored record participating in search.

**Fields**

- `size_t entryId`
- `Key key`
- `Value value`
- `std::vector<std::string> tokens` – tokenized representation of `value`.
- `bool isDeleted` – set on `DEL` until `Cognex::compact()` removes the entry from `entries_` / `postings_`.

**Purpose**

- Encapsulates a key–value pair plus pre‑computed tokens used by the query engine.

---

## **Posting**

Represents token metadata for an entry in the inverted index.

**Fields**

- `size_t entryId` – entry containing the token.
- `size_t frequency` – number of occurrences of the token in that entry.
- `std::vector<size_t> tokenPositions` – token positions within the entry’s token sequence.

**Purpose**

- Enables efficient inverted‑index based queries.

---

## **IndexEntry**

Maps a key to its value location inside the value log.

**Fields**

- `uint64_t offset` – byte offset in the value log.
- `uint32_t valueSize` – stored value size.

**Purpose**

- Enables offset‑based retrieval via `StorageEngine::read_from_log_`.

---

## **RecordHeader**

Header stored before each value‑log record.

**Fields**

- `uint32_t keySize` – key size metadata.
- `uint32_t valueSize` – value size.

---

## **Query / QueryResult / Filter**

High‑level query structures:

- `Filter`
  - `std::string field`
  - `std::string op`
  - `size_t value`
- `Query`
  - `std::vector<std::string> terms`
  - `std::vector<Filter> filters`
  - `size_t topK`
  - Flags and enums:
    - `bool useAnd`, `useOr`, `useNot` (**currently not fully wired in query execution – TODO**)
    - `QueryType type`
    - `Ranking ranking`
    - `SortField sortBy`
- `QueryResult`
  - `size_t entryId`
  - `size_t relevance`
  - `size_t similarity`
  - `double score` (BM25 score field, not currently exposed by the CLI).

These types are consumed by `QueryEngine` and `QueryCommand`. See `docs/query-engine.md` for behavior details.