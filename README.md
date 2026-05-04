# HFT Phase 5: Order Book Performance

Phase 5 starts from a clean slate on top of the Phase 4 build infrastructure.
See `assignments/PHASE05.md` for the spec.

Phase 4 is preserved at the `phase4` git tag; recover any file with
`git checkout phase4 -- <path>`.

## Carried Over

- `include/Timer.hpp` — `chrono` harness for benchmarking
- `include/ObjectPool.hpp` — pool allocator (Phase 5 §3.1)
- CMake + `justfile` + `Makefile` build skeleton
- Catch2 v3 test wiring
- `bench/`, `test/`, `scripts/`, `docs/` directory shells

## Build

Requires LLVM clang++, CMake, Ninja.

```bash
just build      # release configure
just debug      # debug configure
just test       # debug build + Catch2 suite
just bench      # bench harness
just asan       # asan/ubsan build + tests
just clean      # remove build/ and results/*.csv
```

`make` mirrors the same recipes.

## Layout

```
include/   public headers
src/       core implementations
bench/     bench CLI
test/      Catch2 v3 suite
scripts/   report/plot helpers
docs/      design notes and (generated) reports
results/   CSV output (gitignored)
```
