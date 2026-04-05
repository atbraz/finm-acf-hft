# TODO — Phase 1

## Done
- [x] Project scaffolding (CMake, justfile, Makefile)
- [x] `include/kernels.hpp` — declarations + `alloc_aligned`/`free_aligned`
- [x] `tests/correctness.cpp` — test cases for all 4 kernels
- [x] `benchmarks/bench.cpp` — `time_stats()` (mean/stddev), sizes 64/256/512/1024
- [x] `discussions/report.typ` — report template

## Part 1 — Baseline Implementations

- [ ] `multiply_mv_row_major` — implement in `src/kernels.cpp`
- [ ] `multiply_mv_col_major` — implement in `src/kernels.cpp`
- [ ] `multiply_mm_naive` — implement in `src/kernels.cpp`
- [ ] `multiply_mm_transposed_b` — implement in `src/kernels.cpp`
- [ ] `just test` passes all 4 assertions

## Part 2 — Performance Analysis

- [ ] Add unaligned allocation path to `bench.cpp` and compare aligned vs unaligned
- [ ] Run benchmarks at O0, O1, O2, O3 and record results
- [ ] Profile at least one MM kernel (macOS: `xcrun xctrace` or Instruments)
- [ ] Implement at least one optimization (loop reorder, tiling, etc.) in a new kernel variant
- [ ] Benchmark optimized vs baseline and record results

## Part 3 — Discussion Questions (answer in `README.md`)

- [ ] Q1: Pointers vs references in C++
- [ ] Q2: Row-major vs column-major — cache effects with examples from code
- [ ] Q3: CPU caches, temporal/spatial locality, how exploited
- [ ] Q4: Memory alignment — findings from aligned vs unaligned bench
- [ ] Q5: Compiler optimization levels — performance impact, drawbacks
- [ ] Q6: Profiling bottlenecks and how they guided optimization
- [ ] Q7: Teamwork reflection

## Report (`discussions/report.typ` → PDF)

- [ ] Fill in benchmark tables/graphs
- [ ] Include profiler output excerpts or screenshots
- [ ] Document optimization strategy and results
- [ ] `just report` produces final `discussions/report.pdf`
