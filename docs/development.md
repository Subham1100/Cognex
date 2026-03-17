## Development

This document provides a concise guide for building, running, and contributing to Cognex based on the current repository layout.

---

## Build Requirements

From the root `README.md` and `CMakeLists.txt`:

- **CMake** ≥ 3.16
- A **C++17‑compatible compiler** (e.g. `gcc`, `clang`)

No additional third‑party libraries are required beyond the standard C and C++ libraries and system calls used for file IO.

---

## Building from Source

Typical out‑of‑tree CMake build:

```bash
git clone https://github.com/Subham1100/Cognex.git
cd Cognex

cmake -S . -B build
cmake --build build --config Release
```

**TODO:** Document the exact CMake targets for the CLI and benchmark binaries once they are standardized (e.g. `cognex`, `cognex_bench`).

---

## Running the CLI

The CLI binary (installed via `install.sh` or built via CMake) runs an interactive REPL:

- Uses `run_repl(Cognex& db)` from `cli/src/repl.cpp`.
- Commands:
  - `PUT`, `GET`, `DEL`, `QUERY`, `SNAPSHOT`, `HELP`, `EXIT`.
- Data directory:
  - The commented `cli/main.cpp` shows the intended default path `~/.cognex`:
    - `wal.log`
    - `snapshot.dat`
    - `value.log`

Consult the root `README.md` and `docs/guide.md` for up‑to‑date CLI invocation details.

**TODO:** Ensure the active CLI entrypoint and installed binary name are documented once the build configuration is finalized.

---

## Running Benchmarks

The benchmark harness is in `bench/bench.cpp`:

- Constructs a local `Cognex` instance with disk files in the current working directory.
- Runs PUT, GET, and QUERY benchmarks over 10,000 documents.

Example (target name is illustrative only):

```bash
cmake -S . -B build
cmake --build build --config Release

# TODO: replace with the actual benchmark target name
./build/bench
```

Results and methodology are described in `docs/benchmarks.md`.

---

## Code Structure

High‑level layout:

- `engine/`
  - `include/core/` – core types (`Key`, `Value`, `Entry`, `Posting`, `Query`, etc.).
  - `include/storage/` – `StorageEngine`, WAL helpers, snapshot helpers.
  - `include/index/` – `IndexEngine` and tokenization.
  - `include/query/` – `QueryEngine`.
  - `include/debug/` – debug utilities.
  - `src/` – implementations and `cognex.cpp`.
- `cli/`
  - Parser, REPL, command registry, and individual command implementations.
- `bench/`
  - Benchmark harness (`bench.cpp`).
- `docs/`
  - This documentation set.

See `docs/architecture.md` and `docs/overview.md` for conceptual overviews.

---

## Contribution Guidelines

There is no formal CONTRIBUTING file in the repository, but based on the existing code:

- Prefer:
  - Clear, minimal abstractions.
  - Small, focused changes.
  - Consistent naming and formatting with the existing code.
- Keep documentation in sync:
  - If you change engine behavior (e.g. WAL layout, snapshot format, query semantics), update the relevant documents in `/docs`.
  - If you add a new component, add a short section for it under `architecture.md` or a dedicated file if needed.

**TODO:** Add a dedicated `CONTRIBUTING` document if the project grows beyond experimental/educational scope.

