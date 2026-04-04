## CLI and REPL

This document describes the command‑line interface implemented in `cli/`, including parsing, the REPL loop, and available commands.

---

## REPL Loop

The interactive prompt is implemented in `cli/src/repl.cpp`:

- Uses `run_repl(Cognex& db)` to:
  - Print `cognex>` as the prompt.
  - Read lines from `std::cin`.
  - Parse each line into a `ParsedCommand`.
  - Resolve and dispatch the command using `CommandRegistry`.
  - Track write operations and trigger periodic snapshots.

Key points:

- On each successfully parsed command:
  - `ICommand::execute(db, args, write_count)` is called.
  - `write_count` is incremented by commands that perform writes (see command implementations).
- Automatic snapshots in the REPL:

  ```cpp
  constexpr int SNAPSHOT_THRESHOLD = 5;
  ...
  if (write_count >= SNAPSHOT_THRESHOLD) {
      db.snapshot();
      write_count = 0;
      std::cout << "[auto snapshot]\n";
  }
  ```

This is independent of the engine‑level snapshot threshold inside `Cognex`.

---

## Line Parsing

Line parsing is implemented in `cli/src/parser.cpp`:

- `parse_line(std::string_view sv) -> std::optional<ParsedCommand>`
  - Trims leading spaces.
  - Extracts the command name up to the first `"` character (or end of line).
  - Normalizes the command name to uppercase (`to_upper`).
  - Parses every quoted `"..."` segment as a separate argument and adds it to `ParsedCommand::args`.

Example:

```text
PUT   "name"   "Alice is good"
```

Produces:

- `name = "PUT"`
- `args = ["name", "Alice is good"]`

If parsing fails, `parse_line` returns `std::nullopt` and the REPL prints `[ERR parse error]`.

---

## Command Registry

`CommandRegistry` (in `cli/src/command_registry.cpp`) registers concrete command handlers:

- `PUT` → `PutCommand`
- `GET` → `GetCommand`
- `DEL` → `DelCommand`
- `SNAPSHOT` → `SnapshotCommand`
- `HELP` → `HelpCommand`
- `EXIT` → `ExitCommand`
- `QUERY` → `QueryCommand`
- `COMPACT` → `CompactCommand`

On command resolution:

- If the name is recognized, the corresponding command object is returned.
- Otherwise, an `unknown` command handler is used.

---

## Supported Commands

### PUT

**Syntax**

```text
PUT "<key>" "<value>"
```

**Behavior**

- Calls `Cognex::put(Key{key}, Value{value})`.
- Records a `PUT` record in the WAL and appends the value to the value log.
- Updates the in‑memory key index and inverted index.
- Increments `write_count` in the REPL, contributing to automatic snapshots.

---

### GET

**Syntax**

```text
GET "<key>"
```

**Behavior**

- Calls `Cognex::get(Key{key})`.
- If a value exists, prints it.
- If the key is missing, prints `[NIL]`.

---

### DEL

**Syntax**

```text
DEL "<key>"
```

**Behavior**

- Calls `Cognex::del(Key{key})`.
- Records a `DEL` record in the WAL and erases the key from the in‑memory key index.
- Marks the corresponding `Entry` as deleted for search and may trigger `Cognex::compact()` automatically after a fixed number of deletes (see `compactionDeleteThreshold_` in `cognex.h`).
- Does **not** shrink `value.log` on disk; obsolete bytes remain in the append‑only value log.
- Increments `write_count` for REPL auto‑snapshot logic.

---

### COMPACT

**Syntax**

```text
COMPACT
```

**Behavior**

- Calls `Cognex::compact()`.
- Rebuilds in‑memory `entries_`, `postings_`, `keyToEntry_`, and `totalTokens_` by dropping tombstoned entries (`isDeleted`) and reassigning contiguous entry IDs.
- Does not rewrite the WAL, snapshot, or value log; it is an in‑process cleanup of search structures only.
- Does not increment `write_count` (no automatic snapshot side effect from this command alone).

---

### QUERY

**Syntax**

```text
QUERY "<terms>" ["<filter1>"] ["<filter2>"] ...
```

Examples:

- `QUERY "alice"`
- `QUERY "database engine" "relevance >= 2" "top = 20" "sortby similarity"`

**Behavior**

- Parsed by `QueryCommand`:
  - The first argument is split on whitespace into search terms.
  - Remaining arguments are parsed as filters / options (`relevance`, `similarity`, `top`, `sortby`).
- Builds a `Query` object.
- Calls `db.query(query)` and prints matched entries:

  ```text
  Found entries:
    <entryId> key=<key> relevance=<relevance> similarity=<similarity>
  ```

- If no entries match, prints `[NIL]`.

See `query-engine.md` for ranking and filter semantics.

---

### SNAPSHOT

**Syntax**

```text
SNAPSHOT
```

**Behavior**

- Calls `db.snapshot()` (i.e., `Cognex::snapshot()`).
- Persists the current key index via `StorageEngine::snapshot`.
- Truncates the WAL after the snapshot is written.

---

### HELP

**Syntax**

```text
HELP
```

**Behavior**

- Prints available commands and brief usage hints.

---

### EXIT

**Syntax**

```text
EXIT
```

**Behavior**

- Signals the REPL to exit the main loop.

---

## Server

There is **no separate server process or network protocol** in the current codebase—only a local CLI that operates directly on a `Cognex` instance.

See `server.md` for future plans and TODOs.

