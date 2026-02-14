# Architecture

Cognex is a lightweight key–value database with a simple durability model.

---

## Components

- **Command Layer** – Parses and executes user commands
- **Storage Engine** – Manages key–value data
- **Write-Ahead Log (WAL)** – Logs mutations for durability
- **Snapshot Manager** – Saves database state
- **Tokenizer** – Splits values into normalized tokens
- **Inverted Index** – Maps tokens to posting lists
- **Posting Lists** – Track entry frequency and positions

---

## Write Flow

Command → WAL → Storage

---

## Recovery Flow

Snapshot → WAL Replay → Ready

## Posting Structure

Each posting records token metadata per entry:

- entryId – Entry containing the token

- frequency – Number of occurrences

- positions – Token positions inside value