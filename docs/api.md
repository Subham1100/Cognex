# API Reference

This document describes the public interfaces exposed by Cognex.

## Database Commands

`PUT <key> <value>`
Stores or updates a key–value pair.

Creates key if missing

Overwrites value if key exists

`GET <key>`
Retrieves value associated with a key.

Returns value if present

Returns empty result if missing

`DEL <key>`
Deletes a key from storage.

Returns success status

`SNAPSHOT`
Creates a snapshot of the current database state.

`HELP`
Displays supported commands.

`EXIT`
Gracefully shuts down Cognex.

## WAL Functions

`append_and_fsync`
Appends a record to the WAL and ensures durability.

Signature:
```void append_and_fsync(const WalPath& path, const std::string& record);```

Purpose:

Guarantees persistence before applying mutations

`wal_replay`
Replays WAL records sequentially.

Signature:
```bool wal_replay(const WalPath& path, void(*apply)(const std::string_view& record));```

Behavior:

Reads log entries

Invokes callback for each record

Returns success/failure

`wal_truncate`
Clears WAL contents.

Signature:
```void wal_truncate(const WalPath& path);```

Purpose:

Typically used after snapshot creation

## Snapshot Functions

`write_snapshot_atomic`
Writes snapshot safely using atomic replacement.

Signature:
```void write_snapshot_atomic(const SnapshotPath& path, const std::unordered_map<Key,Value>& store);```

Purpose:

Prevents partial/corrupt snapshots

`load_snapshot`
Loads snapshot into memory.

Signature:
```void load_snapshot(const SnapshotPath& path, std::unordered_map<Key,Value>& store);```

Behavior:

Restores persisted database state