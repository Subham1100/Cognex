# Storage Engine

The storage engine manages key–value data.

---

## Data Model

- Unique keys
- String values (current implementation)

---

## Supported Operations

- PUT – Insert / overwrite value
- GET – Retrieve value
- DEL – Remove key

---

## Behavior

- PUT overwrites existing values
- GET returns stored value
- DEL removes key if present

---

## Persistence

Data durability is achieved via:

- WAL
- Snapshots