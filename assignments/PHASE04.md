# HFT Project Phase 4 - Build & Benchmark a High-Frequency Trading System in C++

## Objective

Design and implement a performance-optimized HFT prototype in C++ that:

- Ingests market data
- Accepts and manages client orders
- Matches orders in a fast limit order book
- Reports trades
- Measures and analyzes tick-to-trade latency

Advanced C++ concepts: smart pointers, memory pools, templates, compile-time checks, profiling.

---

## System Architecture

```
+------------------+       +-----------------+       +------------------+
| MarketDataFeed   |  -->  | OrderBook       | <---> | OrderManagement  |
+------------------+       +-----------------+       +------------------+
                                 |     |
                                 v     v
                         +-------------------+      +------------------+
                         | MatchingEngine    | ---> | TradeLogger      |
                         +-------------------+      +------------------+
```

Each module is instrumented to benchmark tick-to-trade latency.

---

## File Structure

```
hft_project/
├── include/
│   ├── MarketData.hpp
│   ├── Order.hpp
│   ├── OrderBook.hpp
│   ├── MatchingEngine.hpp
│   ├── OrderManager.hpp
│   ├── TradeLogger.hpp
│   └── Timer.hpp
├── src/
│   ├── MarketData.cpp
│   ├── OrderBook.cpp
│   ├── MatchingEngine.cpp
│   ├── OrderManager.cpp
│   ├── TradeLogger.cpp
│   └── main.cpp
├── test/
│   └── test_latency.cpp
├── CMakeLists.txt
└── README.md
```

---

## Module Guide

### 1. Market Data Feed Simulator

Simulate ticks with mock price data. Use `alignas(64)` for cache-line alignment and `high_resolution_clock` for timestamps.

```cpp
struct alignas(64) MarketData {
    std::string symbol;
    double bid_price;
    double ask_price;
    std::chrono::high_resolution_clock::time_point timestamp;
};
```

### 2. OrderBook (Template-Based)

Templated `Order<TPrice, TOrderID>`. Store orders in `unique_ptr` with a memory pool allocator. Use `std::multimap` or a cache-friendly flat structure.

```cpp
template <typename PriceType, typename OrderIdType>
struct Order {
    OrderIdType id;
    PriceType price;
    int quantity;
    bool is_buy;
};
```

### 3. Order Management System (OMS)

Track states: New, Cancelled, PartiallyFilled, Filled. Use `shared_ptr` for orders accessed across multiple components. Use `static_assert` to enforce type constraints.

```cpp
static_assert(std::is_integral<OrderIdType>::value, "Order ID must be an integer");
```

### 4. Matching Engine

Match buy/sell orders in the book, return matched trades, optimize for cache locality, and benchmark tick-to-trade latency on every match.

### 5. Trade Logger

Preallocate with `reserve()`, write logs in batches, use RAII for resource management.

---

## Tick-to-Trade Latency

**Definition:** time between receiving a price update (tick) and sending a trade order.

### Measurement

```cpp
using Clock = std::chrono::high_resolution_clock;
auto start = Clock::now();

marketDataHandler.handleTick(tick);
matchingEngine.matchOrder(order);

auto end = Clock::now();
auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
```

Store results in `std::vector<long long>` for analysis.

### Analysis

```cpp
void analyzeLatencies(const std::vector<long long>& latencies) {
    if (latencies.empty()) return;

    auto min    = *std::min_element(latencies.begin(), latencies.end());
    auto max    = *std::max_element(latencies.begin(), latencies.end());
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    double variance = 0.0;
    for (auto l : latencies) variance += (l - mean) * (l - mean);
    double stddev = std::sqrt(variance / latencies.size());

    long long p99 = latencies[static_cast<int>(latencies.size() * 0.99)];

    std::cout << "Tick-to-Trade Latency (ns):\n"
              << "Min: "    << min    << "\n"
              << "Max: "    << max    << "\n"
              << "Mean: "   << mean   << "\n"
              << "StdDev: " << stddev << "\n"
              << "P99: "    << p99    << "\n";
}
```

---

## Experiments

| Variable | Experiment | What to Observe |
|----------|-----------|-----------------|
| Smart vs raw pointers | Swap `unique_ptr` with raw pointers | Memory safety vs overhead |
| Memory alignment | Add/remove `alignas(64)` | Cache behavior differences |
| Custom allocator | Memory pool vs `new`/`delete` | Allocation speed |
| Container layout | Flat array vs `map`/`multimap` | Access time difference |
| Load scaling | 1K, 10K, 100K ticks | Latency consistency under pressure |

---

## Sample Code

### `include/Order.hpp`

```cpp
#pragma once
#include <string>
#include <memory>

template <typename PriceType, typename OrderIdType>
struct Order {
    OrderIdType id;
    std::string symbol;
    PriceType price;
    int quantity;
    bool is_buy;

    Order(OrderIdType id, std::string sym, PriceType pr, int qty, bool buy)
        : id(id), symbol(std::move(sym)), price(pr), quantity(qty), is_buy(buy) {}
};
```

### `include/Timer.hpp`

```cpp
#pragma once
#include <chrono>

class Timer {
public:
    void start() {
        m_start = std::chrono::high_resolution_clock::now();
    }

    long long stop() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count();
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
};
```

### `src/main.cpp`

```cpp
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include "../include/Order.hpp"
#include "../include/Timer.hpp"

using OrderType = Order<double, int>;

int main() {
    std::vector<long long> latencies;
    const int num_ticks = 10000;

    for (int i = 0; i < num_ticks; ++i) {
        Timer timer;
        timer.start();

        OrderType order(i, "AAPL", 150.0 + (i % 5), 100, i % 2 == 0);
        // simulate match logic here

        latencies.push_back(timer.stop());
    }

    auto min    = *std::min_element(latencies.begin(), latencies.end());
    auto max    = *std::max_element(latencies.begin(), latencies.end());
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    std::cout << "Tick-to-Trade Latency (ns):\n"
              << "Min: " << min << " | Max: " << max << " | Mean: " << mean << "\n";
}
```

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.10)
project(HFT_Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(hft_app
    src/main.cpp
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)
```

---

## Deliverables

| Deliverable | Required |
|-------------|----------|
| Modular source code (`.hpp`/`.cpp`) | yes |
| `README.md` with design overview, architecture, build/run instructions | yes |
| Benchmark report (tick-to-trade latency statistics) | yes |
| Performance test results + brief analysis | yes |
| Architecture diagram (system/module/class flow) | yes |
| Video demo | yes |
