## Benchmarks

This document mirrors the root **`BENCHMARKS.md`** at a high level. For the canonical methodology, criteria, and reproduction steps, prefer **`BENCHMARKS.md`** in the repository root.

---

## Harness (`cognex_bench`, `bench/bench.cpp`)

- **Binary:** `cognex_bench` (CMake target).
- **Data directory:** default `bench_data/` (WAL, snapshot, value log). Default behavior wipes the three files before the run, then **`recover()`**.
- **Defaults:** `docs = 10000`, `query_count = 10000`, `seed = 42`.
- **Timer:** `std::chrono::high_resolution_clock` — **wall time** per phase.
- **Throughput:** operations ÷ phase seconds.
- **Time split:** each phase as % of PUT + GET + QUERY total wall time.
- **Memory:** approximate RSS (Linux `VmRSS` or macOS `mach_task_basic_info`), printed as MB.

---

## Criteria (what is being measured)

| Phase | Input | Metric | What dominates |
|-------|--------|--------|----------------|
| PUT | `docs` inserts; keys `key0`…; values = 3–12 random dictionary words | docs/s | WAL, value log, tokenization, postings |
| GET | `query_count` uniform random keys among inserted docs | ops/s | `index_` lookup + value log read |
| QUERY | `query_count` single-term queries, `topK = 10`, random dictionary term | queries/s | Posting lists + BM25 + rank |

Synthetic **dictionary** and **random_sentence** are defined in `bench/bench.cpp`. Not measured: concurrency, network, CLI, mixed interleaved workloads, multi-run statistics.

---

## Sample results (same defaults as root doc)

One reference run (`./build/cognex_bench` defaults):

| Benchmark | Count | Time (s) | Throughput | % of total phase time |
|-----------|-------|----------|------------|------------------------|
| PUT | 10,000 | 0.527247 | ~18,966 docs/s | ~6.65% |
| GET | 10,000 | 0.025308 | ~395,138 ops/s | ~0.32% |
| QUERY | 10,000 | 7.375877 | ~1,356 queries/s | ~93.03% |

RSS (approx): **9 MB**.

---

## How to run

```bash
cmake -S . -B build
cmake --build build --target cognex_bench --config Release
./build/cognex_bench
```

Optional: `./build/cognex_bench <docs> <query_count> <seed> <data_dir>`.

---

## Future work

Capture hardware/OS per run; multiple iterations with median/p95; larger corpora and mixed workloads. See **`BENCHMARKS.md`**.
