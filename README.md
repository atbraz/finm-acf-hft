# HFT Project
---
UChicago FinMath Spring 26 Advanced Computing for Finance HFT Project

**Course:** FINM 32700 - Advanced Computing for Finance

**Team:** 11

**Member:** Antonio Magalhaes Torreao e Braz

## Build

### Prerequisites

- CMake >= 3.20
- [Ninja](https://ninja-build.org)
- LLVM clang++
- [just](https://just.systems) or `make`
- [Typst](https://typst.app) (for report compilation)

### Commands

All commands are available via both `just` and `make` (the `Makefile` mirrors the `justfile`).

| `just` / `make`  | Description                                     |
|-----------------|-------------------------------------------------|
| `build`         | Build all targets (no explicit optimisation)    |
| `test`          | Build and run correctness tests                 |
| `bench`         | Build and run benchmarks (no explicit opt)      |
| `bench-O1`      | Benchmarks at `-O1`                             |
| `bench-O2`      | Benchmarks at `-O2`                             |
| `bench-O3`      | Benchmarks at `-O3`                             |
| `run`           | Build and run main driver                       |
| `report`        | Compile `discussions/report.typ` → PDF          |
| `clean`         | Remove all build directories                    |

