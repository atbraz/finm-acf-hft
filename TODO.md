# TODO -- Phase 1

## Done
- [x] Project scaffolding (CMake, justfile, Makefile)
- [x] `include/kernels.hpp` -- declarations + `alloc_aligned`/`free_aligned`
- [x] `tests/correctness.cpp` -- test cases for all 4 kernels
- [x] `benchmarks/bench.cpp` -- `time_stats()` (mean/stddev), sizes 64/256/512/1024
- [x] `discussions/report.typ` -- report template

## Part 1 -- Baseline Implementations
- [x] `multiply_mv_row_major`
- [x] `multiply_mv_col_major`
- [x] `multiply_mm_naive`
- [x] `multiply_mm_transposed_b`
- [x] `just test` passes all assertions (13 tests + 3000 fuzz trials)

## Part 2 -- Performance Analysis
- [x] Add unaligned allocation path to `bench.cpp` and compare aligned vs unaligned
- [x] Run benchmarks at O0, O1, O2, O3 and record results (`benchmarks/results/`)
- [x] Profile MM kernels (Instruments Time Profiler, CPU Counters, PGO, compiler remarks)
- [x] Implement optimisations: `__restrict__` aliasing + tiled MM (tile=64)
- [x] Benchmark optimised vs baseline and record results

## Part 3 -- Discussion Questions (answer in `README.md`)
- [x] Q1: Pointers vs references in C++
- [x] Q2: Row-major vs column-major -- cache effects with examples from code
- [x] Q3: CPU caches, temporal/spatial locality, how exploited
- [x] Q4: Memory alignment -- findings from aligned vs unaligned bench
- [x] Q5: Compiler optimization levels -- performance impact, drawbacks
- [x] Q6: Profiling bottlenecks and how they guided optimization
- [x] Q7: Teamwork reflection

## Report (`discussions/report.typ` -> PDF)
- [x] Fill in benchmark tables
- [x] Document cache locality analysis
- [x] Document memory alignment findings
- [x] Document compiler optimisation analysis
- [x] Document optimisation strategy and results
- [x] `just report` produces final `discussions/report.pdf`
