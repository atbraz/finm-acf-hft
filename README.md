# HFT Project
---
UChicago FinMath Spring 26 Advanced Computing for Finance HFT Project

**Course:** FINM 32700 - Advanced Computing for Finance
**Team:** 11
**Members:** Antonio Magalhaes Torreao e Braz

## Build

### Prerequisites

- CMake >= 3.20
- [Ninja](https://ninja-build.org)
- LLVM clang++
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

