# HFT Project Phase 1 - High-Performance Linear Algebra Kernels

**Team size:** 1–4 students  
**Submission:** GitHub repository with all source files, `README.md`, and a PDF report.

---

## Part 1: Baseline Implementations

Each team member implements one function using raw pointers. Handle dynamic memory allocation and deallocation appropriately.

### Member 1: Matrix-Vector Multiplication (Row-Major)

```cpp
void multiply_mv_row_major(const double* matrix, int rows, int cols,
                           const double* vector, double* result);
```

### Member 2: Matrix-Vector Multiplication (Column-Major)

```cpp
void multiply_mv_col_major(const double* matrix, int rows, int cols,
                           const double* vector, double* result);
```

### Member 3: Matrix-Matrix Multiplication (Naive)

```cpp
void multiply_mm_naive(const double* matrixA, int rowsA, int colsA,
                       const double* matrixB, int rowsB, int colsB,
                       double* result);
```

`matrixA`, `matrixB`, and `result` are in row-major order.

### Member 4: Matrix-Matrix Multiplication (Transposed B)

```cpp
void multiply_mm_transposed_b(const double* matrixA, int rowsA, int colsA,
                              const double* matrixB_transposed, int rowsB, int colsB,
                              double* result);
```

`matrixA` and `result` are row-major; `matrixB_transposed` is the transpose of B (also row-major).

### Requirements

- Correct mathematical operation
- Allocate/deallocate memory in a test program
- Basic error handling (null pointer checks, dimension compatibility)
- A simple correctness test for each function

---

## Part 2: Performance Analysis and Optimization

### Benchmarking

- Use `std::chrono` to measure execution time
- Test with small, medium, and large square matrices
- Multiple runs per test case; report average and standard deviation
- Present results in a table or graph in the report

### Cache Locality Analysis

- **MV (row-major vs column-major):** explain which access pattern is cache-friendly and why
- **MM (naive vs transposed B):** explain how transposing B improves cache utilization
- Design benchmark cases that highlight stride effects

### Memory Alignment

- Align matrices/vectors to 64-byte boundaries using custom allocators, platform functions, or aligned arrays
- Benchmark aligned vs unaligned versions
- Report whether alignment provided measurable improvement and under what conditions

### Inlining

- Experiment with `inline` on small helper functions
- Compile with `-O0` vs `-O3` (GCC/Clang) or `/Od` vs `/O2` (MSVC)
- Analyze assembly if applicable; discuss when inlining helps or hurts

### Profiling

Use the tool appropriate for your platform:

| Platform | Tool | Notes |
|----------|------|-------|
| Linux | `gprof` | Compile with `-pg`, analyze `gmon.out` |
| Linux | `perf` | `perf stat` for summary, `perf record -g` + `perf report` for call graph |
| macOS | Instruments | Time Profiler template; or `xcrun xctrace` |
| Windows | VS Performance Profiler | Debug → Performance Profiler → CPU Usage |
| Windows | WPT | Use WPR to record, WPA to analyze |
| Windows (Cygwin) | `gprof` | Compile with `-pg` inside Cygwin terminal |

Profile at least one MM implementation (both naive and transposed B). Identify hotspots, relate to cache behavior, and include profiler output excerpts or screenshots in the report.

### Optimization

Based on the analysis above, implement at least one significant optimization:

- Loop reordering
- Blocking/tiling
- Other techniques from class or research

Document the optimization, the reasoning behind it, and benchmark it against the baseline.

---

## Part 3: Discussion Questions (answer in `README.md`)

1. What are the key differences between pointers and references in C++? When would you choose one over the other in numerical algorithms?

2. How does row-major vs column-major storage affect memory access patterns and cache locality in MV and MM multiplication? Give examples from your implementations.

3. How do CPU caches (L1, L2, L3) work? What are temporal and spatial locality, and how did you exploit them in your optimizations?

4. What is memory alignment and why does it matter for performance? Did you observe a difference between aligned and unaligned memory?

5. How did compiler optimization levels affect performance? What are the potential drawbacks of aggressive optimization?

6. What were the main bottlenecks identified by profiling? How did profiling guide your optimization decisions?

7. Reflect on the teamwork: how did dividing implementation tasks and then collaborating on analysis work? What were the challenges and benefits?

---

## Repository Structure

```
project/
├── src/
│   ├── multiply_mv_row_major.cpp
│   ├── multiply_mv_col_major.cpp
│   ├── multiply_mm_naive.cpp
│   ├── multiply_mm_transposed_b.cpp
│   └── main.cpp
├── include/
│   └── kernels.h
├── report.pdf
└── README.md
```

## Submission Checklist

| Item | Required |
|------|----------|
| All `.cpp` and `.h` source files | yes |
| `README.md` with team members, build instructions, discussion answers | yes |
| PDF report with benchmarks, profiler output, optimization analysis | yes |
