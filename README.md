# HFT Phase 3: Local Order Book

Phase 3 implements a local order book and core trading infrastructure. The program reads a text-based market feed, maintains a synchronized view of best bid and ask prices, and places orders when the spread narrows below a configurable threshold. Net position is tracked across fills to alternate between buy and sell orders, keeping exposure balanced.

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

```bash
just build      # configure + build
just run        # build + run hft_phase3
just asan       # build with AddressSanitizer
just clean      # remove build/
```

Requires LLVM clang++ (`/opt/homebrew/opt/llvm/bin/clang++`) and CMake.

## Sample Output

```
[FEED] BID 100.50 x 200
[FEED] ASK 100.52 x 150
[BOOK] spread=0.02, threshold=0.05 -> spread within threshold
[ORDER] placing BUY at 100.50, qty=100
[FEED] EXECUTION 100.50 x 100
[ORDER] fill: BUY 100 @ 100.50, net_position=+100
[FEED] BID 100.53 x 300
[FEED] ASK 100.54 x 200
[BOOK] spread=0.01, threshold=0.05 -> spread within threshold
[ORDER] placing SELL at 100.54, qty=100
```
