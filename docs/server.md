## Server

At present, Cognex does **not** implement a standalone server process or network protocol.

All interaction happens through:

- The in‑process C++ API (`Cognex` implementing `DB` and `Persistence`).
- The CLI / REPL in `cli/`, which runs in the same process and talks directly to a `Cognex` instance.

---

## Current State

- **No TCP/HTTP server:** there is no listener loop, socket handling, or request/response protocol in the repository.
- **No RPC layer:** all commands are executed in‑process via `ICommand` implementations.

Any references to a “server” in future docs or comments should be interpreted as **planned / potential work**, not existing behavior.

---

## Possible Future Direction (Not Implemented)

The following ideas are **not implemented in the current codebase** but may be relevant for future work:

- A simple TCP or HTTP server that:
  - Accepts text commands similar to the CLI.
  - Forwards them to a shared `Cognex` instance.
  - Returns results over the network.
- Concurrency control to handle multiple clients concurrently.
- Authentication, authorization, and multi‑tenant data separation.

These items are intentionally left as **TODOs** and are outside the scope of the existing implementation.

