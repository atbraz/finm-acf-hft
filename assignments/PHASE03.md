# HFT Project Phase 3 - Local Order Book and Core Trading Infrastructure

## Purpose

Build a real-time trading system core that:

- Maintains a real-time market view via a local order book
- Tracks submitted orders and their state (working, filled, canceled)
- Provides memory-safe architecture using smart pointers and RAII
- Serves as the foundation for live Alpaca trading in Phase 4

## Why a Local Order Book?

Your system needs an internal view of the market to make decisions without relying on broker confirmation. This means maintaining:

- Best bid/ask prices and quantities
- A structure updated with each market event
- Instant access to top-of-book price levels

## Key Concepts

- `std::unique_ptr` / `std::shared_ptr` for price level nodes
- RAII for resource lifetimes
- Safe deallocation after cancellation or fill
- No memory leaks or dangling pointers under live data conditions

---

## System Modules

### 1. Market Data Structures (`market_snapshot.h`)

Maintains your internal representation of market state (Level 1, optionally Level 2).

```cpp
struct PriceLevel {
    double price;
    int quantity;
    PriceLevel(double p, int q) : price(p), quantity(q) {}
};

class MarketSnapshot {
    std::map<double, std::unique_ptr<PriceLevel>> bids; // sorted descending
    std::map<double, std::unique_ptr<PriceLevel>> asks; // sorted ascending
};
```

- `bids`: price → quantity, high to low
- `asks`: price → quantity, low to high
- `unique_ptr` handles cleanup when a level is removed

**Interface:**

```cpp
void update_bid(double price, int qty);
void update_ask(double price, int qty);
const PriceLevel* get_best_bid() const;
const PriceLevel* get_best_ask() const;
```

---

### 2. Order Manager (`order_manager.h`)

Tracks all submitted orders, their status, and fill state.

```cpp
enum class OrderStatus { New, Filled, PartiallyFilled, Cancelled };

struct MyOrder {
    int id;
    Side side;
    double price;
    int quantity;
    int filled = 0;
    OrderStatus status = OrderStatus::New;
};

std::map<int, std::unique_ptr<MyOrder>> orders;
```

Orders are removed (and memory reclaimed) on fill or cancellation.

**Interface:**

```cpp
int place_order(Side side, double price, int qty);
void cancel(int id);
void handle_fill(int id, int filled_qty);
void print_active_orders() const;
```

---

### 3. Strategy Execution Loop (`main.cpp`)

Connects market data to order actions.

```cpp
for (auto& update : load_feed("sample_feed.txt")) {
    snapshot.update_bid(update.bid_price, update.bid_qty);
    snapshot.update_ask(update.ask_price, update.ask_qty);

    if (should_trade(snapshot)) {
        int id = om.place_order(Side::Buy, snapshot.get_best_bid()->price, 10);
        std::cout << "Placed BUY order at " << snapshot.get_best_bid()->price << "\n";
    }

    if (update.filled_order_id != -1) {
        om.handle_fill(update.filled_order_id, update.qty);
    }
}
```

No `new` or `delete` required — all ownership through `unique_ptr`.

---

## Required Behaviors

### Market Snapshot Maintenance

Updates best bid/ask on each event. Removes levels when quantity reaches zero.

```
Input:  BID 100.20 300  →  Best bid: 100.20 x 300
Input:  BID 100.20 0    →  Best bid removed
```

### Memory-Safe Price Level Tracking

- `unique_ptr` per `PriceLevel`
- No raw pointers or manual `delete`

### Order Lifecycle

```
Place:     BUY 100.10 x 50  →  ID = 1
Fill:      Order 1 filled 30  →  Partial
Fill:      Order 1 filled 20  →  Filled → removed
```

### Strategy Decision Loop

Example condition: if best bid > 100.00, sell 10 shares. Must check, place, and respond to fills.

### No Memory Leaks

Verify with:

```bash
g++ -fsanitize=address ...   # AddressSanitizer
valgrind ./your_program      # Linux
```

---

## Memory Safety Rules

**Do:**
- Use RAII for all resource lifetimes
- Store heap objects in `unique_ptr` (or `shared_ptr` only if shared ownership is needed)
- Remove elements from containers when no longer active

**Do not:**
- Use `new` / `delete` directly
- Leave unused objects in memory
- Access invalidated memory (use-after-free)
- Double-delete anything

---

## Feed Parser (`feed_parser.h`)

```cpp
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

enum class FeedType { BID, ASK, EXECUTION, UNKNOWN };

struct FeedEvent {
    FeedType type = FeedType::UNKNOWN;
    double price = 0.0;
    int quantity = 0;
    int order_id = -1;

    void print() const {
        switch (type) {
            case FeedType::BID:       std::cout << "[BID] " << price << " x " << quantity << "\n"; break;
            case FeedType::ASK:       std::cout << "[ASK] " << price << " x " << quantity << "\n"; break;
            case FeedType::EXECUTION: std::cout << "[EXECUTION] Order " << order_id << " filled: " << quantity << "\n"; break;
            default:                  std::cout << "[UNKNOWN]\n";
        }
    }
};

std::vector<FeedEvent> load_feed(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<FeedEvent> events;

    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << "\n";
        return events;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "BID") {
            double price; int qty;
            if (iss >> price >> qty) events.push_back({FeedType::BID, price, qty});
        } else if (type == "ASK") {
            double price; int qty;
            if (iss >> price >> qty) events.push_back({FeedType::ASK, price, qty});
        } else if (type == "EXECUTION") {
            int order_id, filled;
            if (iss >> order_id >> filled) events.push_back({FeedType::EXECUTION, 0.0, filled, order_id});
        } else {
            std::cerr << "Unknown event type: " << line << "\n";
        }
    }

    return events;
}
```

**Usage in `main.cpp`:**

```cpp
#include "feed_parser.h"

int main() {
    auto feed = load_feed("sample_feed.txt");

    for (const auto& event : feed) {
        event.print();

        if (event.type == FeedType::BID)
            snapshot.update_bid(event.price, event.quantity);
        else if (event.type == FeedType::ASK)
            snapshot.update_ask(event.price, event.quantity);
        else if (event.type == FeedType::EXECUTION)
            order_manager.handle_fill(event.order_id, event.quantity);
    }

    return 0;
}
```

---

## Sample Feed (`sample_feed.txt`)

```
# Initial market setup
BID 100.10 300
ASK 100.20 250

BID 100.15 200
ASK 100.18 100
ASK 100.18 60
BID 100.10 0
ASK 100.18 30
ASK 100.18 0
ASK 100.25 200

EXECUTION 1 10
EXECUTION 1 20

BID 100.17 100

EXECUTION 2 50

BID 100.15 100
ASK 100.22 150

EXECUTION 2 50

BID 100.20 300
ASK 100.23 100
```

---

## Sample Output (`output.log`)

```
[Market] Best Bid: 100.10 x 300
[Market] Best Ask: 100.20 x 250
[Market] Best Bid: 100.10 removed
[Market] New Ask: 100.25 x 200

[Strategy] Placing SELL order at 100.20 x 50 (ID = 1)
[Execution] Order 1 filled: 10
[Order] Order 1 partially filled: 10 / 50
[Execution] Order 1 filled: 40
[Order] Order 1 completed (50 / 50) and removed
```

---

## Deliverables

| File | Description |
|------|-------------|
| `market_snapshot.h/.cpp` | Bid/ask level tracking with `unique_ptr` |
| `order_manager.h/.cpp` | Order placement, fill, cancellation |
| `feed_parser.h` | Feed file parser |
| `main.cpp` | Strategy loop |
| `sample_feed.txt` | Simulated market feed |
| `output.log` | (optional) Program action log |
| `README.md` | Architecture, memory management, compile/run instructions |

---

## Evaluation Checklist

| Requirement | Criteria |
|-------------|----------|
| Market snapshot updates | Correctly tracks best bid/ask |
| Memory-safe order book | Uses `unique_ptr`, no leaks |
| Smart pointer usage | No raw `new` or `delete` |
| Order tracking system | Correctly tracks, fills, and cancels |
| Strategy loop | Places orders based on logic |
| Clean output logs | Human-readable trade info |
| `README.md` | Architecture, how to run, how it works |
