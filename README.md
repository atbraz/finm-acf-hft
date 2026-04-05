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
| `multiply_mv_row_major` | Matrix-vector multiply (row-major matrix) | Baseline: stride-1 access on the matrix |
| `multiply_mv_col_major` | Matrix-vector multiply (column-major matrix) | Swaps loop order to keep stride-1 on column-stored data |
| `multiply_mm_naive` | Matrix-matrix multiply (both row-major) | Triple loop baseline: strided access on B's columns |
| `multiply_mm_transposed_b` | Matrix-matrix multiply (B pre-transposed) | Eliminates B's column stride by transposing ahead of time |
| `multiply_mm_tiled` | Tiled/blocked matrix-matrix multiply | Blocks sub-matrices to fit in L1/L2 cache (default tile=32) |

Aligned-memory helpers (`alloc_aligned<T>` / `free_aligned`) provide 64-byte aligned allocations via `std::aligned_alloc`.

## Project Structure

```
include/kernels.hpp     kernel declarations + aligned memory helpers
src/kernels.cpp         kernel implementations
src/main.cpp            scratch driver
tests/correctness.cpp   assert-based tests (single binary, all four kernels)
benchmarks/bench.cpp    chrono benchmarks: mean/stddev over 100 runs, n in {64,256,512,1024}
scripts/bench_all.py    run benchmarks across all opt levels, collect results
scripts/sysinfo.sh      capture system details for reproducibility
discussions/report.typ   Typst report source
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
| `profile-pgo` | LLVM instrumented profiling: per-function call counts and block frequencies |
| `profile-remarks` | Compiler vectorization and optimization diagnostics (`-Rpass`) |
| `profile-time` | `time -l` across all opt levels: RSS, page faults, wall time |
| `profile-dtrace` | Hardware event sampling via DTrace (requires sudo) |
| `profile-xctrace-counters` | Instruments CPU Counters: L1/L2 cache misses, branch mispredictions |
| `profile-xctrace-time` | Instruments Time Profiler: detailed call tree |
| `profile-xctrace` | Run both Instruments profiles |
| `profile-all` | Run all CLI profilers (text output) + sysinfo |

#### Report

| Command | Description |
|---|---|
| `report` | Compile `discussions/report.typ` to PDF via Typst |

## Discussion Questions

### Q1: Pointers vs References

Pointers are nullable, rebindable, and support arithmetic. References are non-null aliases that cannot be reseated after initialization.

In numerical code, raw pointers can be the optimal choice for dynamically sized arrays. All five kernels take `const double*` parameters because the data is heap-allocated with a runtime-determined size, and pointers compose with `std::aligned_alloc` and `new[]`. References are preferred for scalar parameters and return values where null is meaningless.

The `__restrict__` qualifier is pointer-only (it cannot be applied to references), which is another reason our kernel signatures use pointers: it lets us promise the compiler that output and input buffers don't alias, enabling vectorisation.

### Q2: Row-Major vs Column-Major: Cache Effects

Row-major stores elements of each row contiguously. Column-major stores each column contiguously.

**Matrix-vector:** In `multiply_mv_row_major`, the inner loop walks `matrix[i*cols + j]` with stride 1, giving sequential cache-line access. In `multiply_mv_col_major`, the inner loop walks `matrix[j*rows + i]` with stride 1 through a single column (contiguous in column-major layout), while `vector[j]` is loop-invariant and hoisted. Both achieve good spatial locality. At O2, col-major (0.13 ms at n=1024) is faster than row-major (0.48 ms) because the compiler reduces the inner loop to a scaled vector accumulation.

**Matrix-matrix:** In `multiply_mm_naive`, the inner loop accesses `matrixB[k*colsB + j]` with stride `colsB`; each step jumps an entire row, thrashing L1. In `multiply_mm_transposed_b`, both A and B^T have stride-1 access over `k`, turning the inner loop into a dot product of two contiguous rows. This eliminates the column-stride penalty and yields a consistent 2x speedup.

### Q3: CPU Caches and Locality

CPU caches are small, fast SRAM buffers between the CPU and main memory, organised in a hierarchy: L1 (128 KB per P-core on M4 Pro, ~1 cycle), L2 (16 MB, ~10 cycles), then DRAM (~100+ cycles). Data is transferred in cache lines (128 bytes on M4 Pro).

- **Spatial locality:** accessing nearby addresses. Stride-1 loops exploit this; one cache-line fetch serves 16 consecutive doubles.
- **Temporal locality:** reusing recently accessed data. Tiling exploits this; a 64x64 tile of A is loaded once and reused across 64 columns of B before eviction.

Our tiled kernel sizes the working set (3 x 64^2 x 8 = 96 KB) to fit within L1d (128 KB), maximising reuse. The naive kernel has no temporal reuse of B: each (i,j) pair loads a different column, making total B traffic O(n^3).

### Q4: Memory Alignment

Memory alignment means placing data at addresses that are multiples of a given boundary. 64-byte alignment ensures that arrays start at cache-line boundaries, preventing a single element from straddling two cache lines.

**Findings:** At large n (>= 256), aligned vs unaligned differences were within noise (<= 2%). Apple M4 Pro handles unaligned loads in hardware with near-zero penalty. At small n (= 64), alignment showed up to 54% improvement for `mm_transposed_b`, but these are sub-0.1 ms operations where the absolute difference is tens of microseconds.

### Q5: Compiler Optimisation Levels

| Transition | Effect | Why |
|---|---|---|
| O0 to O1 | 3 to 4x (MM kernels) | Inlining, register allocation, constant propagation, dead-code elimination |
| O1 to O2 | 1 to 5x (kernel-dependent) | Loop vectorisation (NEON width 2, interleave 4), aggressive LICM |
| O2 to O3 | ~1x (within noise) | Aggressive unrolling, function specialisation |

O2 is the sweet spot for our kernels: it enables vectorisation without the code-size bloat of O3. At n=512, `mm_naive` is 111 ms at both O1 and O2, but O3 can regress due to over-aggressive unrolling increasing instruction-cache pressure (M4 Pro L1i is 192 KB).

### Q6: Profiling Bottlenecks and Optimisation Decisions

Profiling tools used: Instruments Time Profiler, CPU Counters, LLVM PGO, and compiler vectorisation remarks (`-Rpass` / `-Rpass-missed`).

**Bottleneck 1: Aliasing blocked vectorisation.** `-Rpass-missed=slp-vectorizer` reported "vectorization was possible but not beneficial" on all inner loops. Root cause: without `__restrict__`, the compiler couldn't prove stores to `result` don't alias reads from `matrixA`/`matrixB`. Fix: added `__restrict__` to all pointer parameters. Result: all five inner loops vectorise at width 2, interleave 4.

**Bottleneck 2: Cache misses in naive MM.** Time Profiler showed 95%+ of execution time in the naive k-loop. CPU Counters confirmed high L1 miss rates from the stride-`colsB` access on B. Fix: tiled MM with tile=64, sizing the working set (96 KB) to fit L1d (128 KB). Result: 4.9x speedup over naive at n=1024.

### Q7: Teamwork Reflection

This was a solo project with significant assistance from Claude, and the workflow worked well. Claude handled boilerplate and iterative refinement, the full test and benchmark suite were run after every change.
