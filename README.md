# HFT Phase 3: Local Order Book


## Architecture

```
sample_feed.txt
      |
      v
 feed_parser.h
      |
      v
   main.cpp -----> MarketSnapshot (best bid/ask, price levels)
      |
      v
 OrderManager (order lifecycle: open, partial fill, complete, cancel)
```

## Memory Management

All heap allocation uses RAII via `std::unique_ptr`. There are no raw `new` or `delete` calls. Each `PriceLevel` in the order book and each `MyOrder` in the order manager is owned by a `unique_ptr`, so resources are released automatically when entries are removed or the program exits. Verified clean under AddressSanitizer.

## Build and Run

With `just` (recommended) or `make`:

```bash
just build      # configure + build
just run        # build + run hft_phase3
just asan       # build with AddressSanitizer
just clean      # remove build/
```

```bash
make build      # same recipes available via make
make run
make asan
make clean
```

Requires LLVM clang++, CMake, and Ninja.

## Sample Output

```
[BID] 100.1 x 300
[Market] Best Bid: 100.10 x 300
[ASK] 100.20 x 250
[Market] Best Ask: 100.20 x 250
[BID] 100.15 x 200
[Market] Best Bid: 100.15 x 200
[Strategy] Spread 0.05 < threshold 0.10. Placing SELL at 100.20 x 10 (ID = 1, pos = 0)
[Execution] Order 1 filled: 5
[Order] Order 1 partially filled: 5 / 10
[Execution] Order 1 filled: 5
[Order] Order 1 completed (10 / 10) and removed
```
