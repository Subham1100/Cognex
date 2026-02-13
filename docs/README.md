# Cognex

Cognex is a lightweight **key–value database** designed with a focus on **simplicity, clarity, and crash-safe persistence**.

It is built as an educational and experimental system to explore core database concepts such as **storage engines**, **Write-Ahead Logging (WAL)**, and **snapshot-based recovery**.

---

## Overview

Cognex is a minimal database implementation supporting essential key–value operations while demonstrating real-world **durability** and **recovery mechanisms**.

This project prioritizes understanding over abstraction, making database internals easy to study and extend.

---

## Key Features

- **Basic key–value storage**
- **Deterministic write path**
- **Write-Ahead Log (WAL) durability**
- **Snapshot-based recovery**
- **Readable and understandable internals**

---

## Design Philosophy

Cognex prioritizes:

- **Simplicity over feature bloat**
- **Clarity over complexity**
- **Predictable crash recovery**
- **Clean separation of components**

---

## Core Components

### **Storage Engine**
Manages key–value data in memory and coordinates persistence.

### **Write-Ahead Log (WAL)**
Ensures durability by recording mutations before applying them.

### **Snapshot Manager**
Periodically persists full database state for faster recovery.

---

## Current Scope

- String keys  
- String values  
- `PUT`, `GET`, `DEL` operations  
- WAL-based durability  
- Snapshot-based recovery  

