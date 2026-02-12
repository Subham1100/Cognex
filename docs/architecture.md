# Architecture

Cognex is a lightweight key–value database with a simple durability model.

---

## Components

- **Command Layer** – Parses and executes user commands
- **Storage Engine** – Manages key–value data
- **Write-Ahead Log (WAL)** – Logs mutations for durability
- **Snapshot Manager** – Saves database state

---

## Write Flow

Command → WAL → Storage

---

## Recovery Flow

Snapshot → WAL Replay → Ready