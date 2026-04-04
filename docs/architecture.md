# Architecture

Cognex is a lightweight persistent key–value database designed around a log-structured storage model and a WAL-based durability strategy.

## Core Components

- Command Layer
Parses and executes user commands (`PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `COMPACT`, …)

- Storage Engine
Coordinates writes, reads, and indexing

- Write-Ahead Log (WAL)
Ensures durability by recording mutations before application

- ValueLog
Append-only storage for values

- Index Layer
Maps keys → {offset, valueSize} inside the ValueLog

- Snapshot Manager
Periodically persists index state for faster recovery

- Tokenizer
Normalizes and splits values into tokens

- Inverted Index
Maps tokens → posting lists

- Posting Lists
Track token metadata per entry

## Write Flow
~~~
Command
   ↓
WAL append (+ fsync batching)
   ↓
ValueLog append
   ↓
Index update (key → offset)
   ↓
Tokenization + Inverted Index update
~~~

## Read Flow
~~~
GET
   ↓
Index lookup (key → offset)
   ↓
pread(ValueLog, offset)
Recovery Flow
Load Snapshot
   ↓
Replay WAL
   ↓
Rebuild Index / State
   ↓
Ready
~~~

## Durability Strategy

- Every mutation is first recorded in the WAL
- WAL fsync is batched for performance
- ValueLog uses append-only writes
- Snapshots reduce WAL replay time

## Posting Structure

Each posting records token metadata per entry:

- entryId – Entry containing the token
- frequency – Number of token occurrences
- positions – Token positions within the value