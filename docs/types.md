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

## **Hash Support**

`Key` provides a `std::hash<Key>` specialization.

**Purpose**

- Enables use in hash-based containers
- Hash derived from underlying string