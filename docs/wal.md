# Write-Ahead Log (WAL)

The WAL ensures durability and crash recovery.

---

## Principle

Log first → Apply later

---

## Logged Operations

- PUT
- DEL

---

## Recovery

On restart:

1. Load snapshot (if present)
2. Replay WAL entries

---

## Goal

Prevent data loss during crashes