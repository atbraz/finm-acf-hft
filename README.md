# HFT Project
---
UChicago FinMath Spring 26 Advanced Computing for Finance HFT Project

**Course:** FINM 32700 - Advanced Computing for Finance

**Team:** 11

**Member:** Antonio Magalhaes Torreao e Braz

## Kernels

Five linear-algebra kernels, each isolating a specific memory-access pattern:

| Kernel | What it does | Why |
|---|---|---|
| `multiply_mv_row_major` | Matrix-vector multiply (row-major matrix) | Baseline — stride-1 access on the matrix |
| `multiply_mv_col_major` | Matrix-vector multiply (column-major matrix) | Swaps loop order to keep stride-1 on column-stored data |
| `multiply_mm_naive` | Matrix-matrix multiply (both row-major) | Triple loop baseline — strided access on B's columns |
| `multiply_mm_transposed_b` | Matrix-matrix multiply (B pre-transposed) | Eliminates B's column stride by transposing ahead of time |
| `multiply_mm_tiled` | Tiled/blocked matrix-matrix multiply | Blocks sub-matrices to fit in L1/L2 cache (default tile=32) |

Aligned-memory helpers (`alloc_aligned<T>` / `free_aligned`) provide 64-byte aligned allocations via `std::aligned_alloc`.

## Project Structure

```
include/kernels.hpp    — kernel declarations + aligned memory helpers
src/kernels.cpp        — kernel implementations
src/main.cpp           — scratch driver
tests/correctness.cpp  — assert-based tests (single binary, all four kernels)
benchmarks/bench.cpp   — chrono benchmarks: mean/stddev over 100 runs, n in {64,256,512,1024}
scripts/bench_all.py   — run benchmarks across all opt levels, collect results
scripts/sysinfo.sh     — capture system details for reproducibility
discussions/report.typ  — Typst report source
```

## Build

### Prerequisites

- CMake >= 3.20
- [Ninja](https://ninja-build.org)
- LLVM clang++ (`/opt/homebrew/opt/llvm/bin/clang++`)
- [just](https://just.systems) or `make`
- [Typst](https://typst.app) (for report compilation)

### Commands

All commands are available via both `just` and `make` (the `Makefile` mirrors the `justfile`). Each optimization level builds into its own directory (`build/`, `build-O1/`, etc.) so results can be compared without rebuilding.

#### Build and Run

| Command | Description |
|---|---|
| `build` | Build all targets (no explicit opt flags) |
| `build-O1` / `build-O2` / `build-O3` | Build at the given optimization level |
| `build-all` | Build all four optimization levels |
| `run` | Build and run main driver |
| `run-O1` / `run-O2` / `run-O3` | Run main at the given optimization level |
| `test` | Build and run correctness tests |
| `clean` | Remove all build directories (including `build-pgo`) |

#### Benchmarks

| Command | Description |
|---|---|
| `bench` | Benchmarks with no explicit opt flags |
| `bench-O1` / `bench-O2` / `bench-O3` | Benchmarks at the given optimization level |
| `bench-all` | Run benchmarks across all opt levels via `scripts/bench_all.py` |

#### Profiling

All profiling output goes to `profile/`. Run `profile-all` to generate all text-based profiles at once.

| Command | Description |
|---|---|
| `profile-sysinfo` | Capture system details (CPU, memory, OS) for reproducibility |
| `profile-sample` | Wall-clock call-tree sampling via macOS `sample` (no sudo) |
| `profile-pgo` | LLVM instrumented profiling — per-function call counts and block frequencies |
| `profile-remarks` | Compiler vectorization and optimization diagnostics (`-Rpass`) |
| `profile-time` | `time -l` across all opt levels — RSS, page faults, wall time |
| `profile-dtrace` | Hardware event sampling via DTrace (requires sudo) |
| `profile-xctrace-counters` | Instruments CPU Counters — L1/L2 cache misses, branch mispredictions |
| `profile-xctrace-time` | Instruments Time Profiler — detailed call tree |
| `profile-xctrace` | Run both Instruments profiles |
| `profile-all` | Run all CLI profilers (text output) + sysinfo |

#### Report

| Command | Description |
|---|---|
| `report` | Compile `discussions/report.typ` to PDF via Typst |

