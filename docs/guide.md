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

## Commands

### **PUT `<key>` `<value>`**

Stores a value under the specified key.

- If the key does **not exist** → creates a new entry
- If the key **exists** → overwrites the value

**Example**

PUT name Alice
PUT age 20


---

### **GET `<key>`**

Retrieves the value associated with a key.

- Returns the stored value if key exists
- Returns an error / null if key does not exist

**Example**

GET name

**Output**

Alice


---

### **DEL `<key>`**

Deletes a key and its value from storage.

- No-op if key does not exist

**Example**

DEL age


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

### **HELP**

Displays available commands.

---

### **EXIT**

Gracefully shuts down Cognex.

---

## 🧠 Storage Behavior

### **Key–Value Model**

- Keys are unique
- Values are stored as raw strings (current implementation)

---

### **Overwrite Semantics**

`PUT` always replaces existing values.

PUT x 10
PUT x 20
GET x → 20


---

### **Persistence**

Depending on your current implementation, Cognex may persist data using:

- Write-Ahead Log (WAL)
- Snapshot files

See:

- [WAL](wal.md)
- [Snapshotting](snapshot.md)

---

## ⚠️ Current Limitations

- Values are treated as strings
- No secondary indexes
- No range queries
- No TTL / expiration
- No transactions 

---

## 🚀 Future Enhancements (Planned)

- Typed values
- Indexing
- MVCC / transactions
- Compression
- Page-based storage

---

## ✅ Example Session
