# High-Performance Linear Algebra Kernels

**Course:** ACF
**Team:** 11
**Members:** [Your Name]

## Build

### Prerequisites

- CMake >= 3.20
- [Ninja](https://ninja-build.org)
- LLVM clang++ via Homebrew: `/opt/homebrew/opt/llvm/bin/clang++`
- [just](https://just.systems)
- [Typst](https://typst.app) (for report compilation)

### Commands

| Command         | Description                                     |
|----------------|-------------------------------------------------|
| `just build`   | Build all targets (no explicit optimisation)    |
| `just test`    | Build and run correctness tests                 |
| `just bench`   | Build and run benchmarks (no explicit opt)      |
| `just bench-O1`| Benchmarks at `-O1`                             |
| `just bench-O2`| Benchmarks at `-O2`                             |
| `just bench-O3`| Benchmarks at `-O3`                             |
| `just run`     | Build and run main driver                       |
| `just report`  | Compile `discussions/report.typ` → PDF          |
| `just clean`   | Remove all build directories                    |

## Discussion Questions

1. **Pointers vs References in C++**

   _To be completed after Part 2 analysis._

2. **Row-major vs Column-major and Cache Locality**

   _To be completed after Part 2 analysis._

3. **CPU Caches and Locality (L1/L2/L3, temporal and spatial locality)**

   _To be completed after Part 2 analysis._

4. **Memory Alignment**

   _To be completed after Part 2 analysis._

5. **Compiler Optimisations and Inlining**

   _To be completed after Part 2 analysis._

6. **Profiling Bottlenecks**

   _To be completed after Part 2 analysis._

7. **Teamwork Reflection**

   _To be completed after Part 2 analysis._
