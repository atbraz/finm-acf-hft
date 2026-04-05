# HFT Project Phase 2 – Momentum-Based Smart Order Client

## Overview

Simulate an HFT client that connects to a server, receives streaming prices, and sends orders. Only the first client to respond to each price wins.

**Goal:** implement a momentum-based strategy that sends orders only when momentum is detected.

## Files Provided

- `hft_server.cpp` — broadcasts a new price every 5 seconds
- `hft_client.cpp` — receives prices and optionally sends orders

## Step 1: Compile and Test

```bash
g++ -std=c++20 hft_server.cpp -o hft_server -pthread
g++ -std=c++20 hft_client.cpp -o hft_client
```

Run in separate terminals:

```bash
./hft_server
./hft_client
```

Verify the client connects, prices print every 5 seconds, and at least one client sends orders.

## Step 2: Implement Momentum Strategy

Modify `hft_client.cpp`, specifically:

```cpp
void receiveAndRespond(int socketFd, const string& name)
```

### 1. Parse incoming messages

Messages follow the format `price_id,price`, e.g. `3,105.6`. Use `find(',')`, `stoi()`, and `stof()` to parse.

### 2. Store the last 3 prices

```cpp
std::deque<float> priceHistory;

if (priceHistory.size() >= 3)
    priceHistory.pop_front();
priceHistory.push_back(currentPrice);
```

### 3. Detect momentum

```cpp
if (priceHistory.size() == 3) {
    float a = priceHistory[0];
    float b = priceHistory[1];
    float c = priceHistory[2];

    bool up   = (a < b) && (b < c);
    bool down = (a > b) && (b > c);

    if (up || down) {
        // HIT
    }
}
```

### 4. Send order on momentum

Use `send()` with the current `price_id` and simulate reaction latency:

```cpp
this_thread::sleep_for(chrono::milliseconds(10 + rand() % 50));
```

### 5. Log decisions

```cpp
cout << "Momentum up! Sending order for price ID " << priceId << endl;
cout << "No momentum. Ignoring price ID " << priceId << endl;
```

> No need to modify socket code or `hft_server.cpp` — focus only on decision logic.

## Deliverables

| File | Description |
|------|-------------|
| `hft_client.cpp` | Modified client with momentum logic |
| `strategy.md` | 1–2 paragraph explanation of decision logic |
| `demo.mp4` / `demo.mov` | 1–2 min recording showing prices received and orders sent only on momentum |

## Grading

| Criteria | Points |
|----------|--------|
| Client compiles and connects | 5 |
| Momentum detection works as expected | 10 |
| Uses STL correctly (`deque`, logic) | 5 |
| Logging and decision clarity | 5 |
| `strategy.md` explanation | 5 |
| Demo video included | 5 |
| Bonus: hit stats / clean design | +5 |

## Learning Objectives

- C++ classes, STL containers, and functions
- Event-based logic in an HFT-style system
- Responding to streamed data in real-time
- Benchmarking and analyzing simple algorithm behavior
