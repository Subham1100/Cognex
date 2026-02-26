# Types

Core data structures used in Cognex.

---

## **WalPath**

Represents the filesystem location of the Write-Ahead Log.

**Fields**

- `value` – Path string

**Purpose**

- Encapsulates WAL file path

---

## **SnapshotPath**

Represents the filesystem location of a snapshot.

**Fields**

- `value` – Path string

**Purpose**

- Encapsulates snapshot file path

---

## **Key**

Represents a unique identifier for stored data.

**Fields**

- `value` – Key string

**Behavior**

- Equality comparison supported
- Hashable (std::hash specialization)

---

## **Value**

Represents stored data.

**Fields**

- `value` – Value string

---

## **Entry**

Represents a stored database record.

**Fields**

- `id` – Entry identifier
- `key` – Key object
- `value` – Value object
- `tokens` – Tokenized metadata

**Purpose**

- Encapsulates a key–value pair with metadata

---

## **Posting**

Represents token metadata for an entry.

**Fields**

- `entryId` – Entry containing the token
- `frequency` – Number of occurrences
- `tokenPositions` – Positions within the value

**Purpose**

- Enables inverted index queries


---
## **IndexEntry**

Maps a key to its value location inside the ValueLog.

**Fields**

- `offset` – Byte offset in ValueLog
- `valueSize` – Stored value size

**Purpose**

- Enables offset-based retrieval via pread()

---

## **RecordHeader**

Header stored before ValueLog records.

**Fields**

- `keySize` – Key size metadata

- `valueSize` – Value size

---