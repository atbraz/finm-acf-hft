# HFT Phase 4: Latency-Instrumented Prototype

Performance-instrumented HFT prototype in C++23. Synthetic market-data feed,
matching engine, OMS, hand-rolled memory pool, and a benchmark harness that
measures tick-to-trade latency under five compile-time variants.

## Architecture

```
+-----------------+        +-----------------+        +-----------------+
| MarketDataFeed  | -----> |  OrderBook      | <----> | OrderManager    |
+-----------------+        +-----------------+        +-----------------+
                                  |    ^                     ^
                                  v    |                     |
                          +-----------------+    +-----------------+
                          | MatchingEngine  |--> | TradeLogger     |
                          +-----------------+    +-----------------+
                                  |
                                  v
                          +-----------------+
                          | latency vector  |
                          +-----------------+
```

Full diagram and lifecycle notes: [`docs/architecture.md`](docs/architecture.md).

## Build and Run

Requires LLVM clang++, CMake, Ninja, Python 3 (for the report renderer).

```bash
just build      # release build of hft_app, hft_bench, hft_test
just test       # debug build + run Catch2 test suite
just run        # demo run, prints latency stats
just bench      # single reference bench run
just bench-all  # full variant matrix -> results/all_runs.csv
just asan       # address+undefined sanitizer build + tests
just clean      # remove build/ and results/*.csv
```

`make` recipes mirror the `just` ones. CMake builds directly:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/hft_app
```

## Benchmarks

After `just bench-all`, render the report:

```bash
python3 scripts/render_report.py
```

This writes `docs/benchmark_report.md` with one table per experiment plus a
notes section. The five experiments:

| Experiment | Variants | Build defines |
|------------|----------|---------------|
| Pointer ownership | reference / raw-ptr | `HFT_USE_RAW_PTR` |
| Cache alignment   | reference / no-align | `HFT_ALIGN_CACHE` |
| Allocator         | reference / no-pool  | `HFT_USE_POOL` |
| Book container    | reference / book-mmap | `HFT_BOOK_IMPL_FLAT` |
| Load scaling      | 1k / 10k / 100k ticks | `--ticks` |

## Results

Demo run (`./build/hft_app 10000 42`, Apple Silicon Release):

```
Tick-to-Trade Latency (ns) over 10000 ticks
  Min:    0
  Mean:   4349.82
  Stddev: 19521.02
  p50:    3958
  p95:    8667
  p99:    12584
  Max:    1928083
```

Variant matrix (`bash bench/run_all.sh`, 10k ticks each, seed 42):

```
[reference]  mean=2898ns  p99=7750ns
[raw-ptr]    mean=2771ns  p99=7000ns
[no-align]   mean=2832ns  p99=8000ns
[no-pool]    mean=2336ns  p99=5625ns
[book-mmap]  mean=508ns   p99=625ns
[load-1k]    mean=672ns   p99=1917ns
[load-10k]   mean=2432ns  p99=5917ns
[load-100k]  mean=12273ns p99=42167ns
```

The multimap book is the surprise: at 10k ticks it beats the flat sorted vector
because the flat side pays an O(n) shift on every level insert and erase, while
the multimap is amortized O(log n). Pointer ownership, cache alignment, and
pool toggles all sit inside run-to-run noise at this scale. Load scaling looks
roughly linear from 1k to 100k ticks.

## Memory Safety

- Orders are owned by `OrderManager` via `std::shared_ptr<OrderT>`.
- `OrderBook` stores non-owning `OrderT*` pointers.
- `ObjectPool` reuses slots; `PoolDeleter` returns slots when the last
  `shared_ptr` is dropped.
- Run `just asan` to verify clean under AddressSanitizer + UBSan.

## Project Layout

```
include/   public headers (header-only Timer, Order, ObjectPool, Config)
src/       core .cpp implementations + main demo
bench/     bench CLI + matrix driver script
test/      Catch2 v3 suite
scripts/   report renderer
docs/      architecture and (generated) benchmark report
results/   CSV output (gitignored)
```
