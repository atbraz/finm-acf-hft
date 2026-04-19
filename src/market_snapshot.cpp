#include "market_snapshot.h"

#include <limits>

void MarketSnapshot::update_bid(double price, int qty) {
    if (qty <= 0) {
        bids_.erase(price);
        return;
    }
    auto it = bids_.find(price);
    if (it != bids_.end()) {
        it->second->quantity = qty;
    } else {
        bids_.emplace(price, std::make_unique<PriceLevel>(price, qty));
    }
}

void MarketSnapshot::update_ask(double price, int qty) {
    if (qty <= 0) {
        asks_.erase(price);
        return;
    }
    auto it = asks_.find(price);
    if (it != asks_.end()) {
        it->second->quantity = qty;
    } else {
        asks_.emplace(price, std::make_unique<PriceLevel>(price, qty));
    }
}

const PriceLevel* MarketSnapshot::get_best_bid() const {
    if (bids_.empty()) return nullptr;
    return bids_.begin()->second.get();
}

const PriceLevel* MarketSnapshot::get_best_ask() const {
    if (asks_.empty()) return nullptr;
    return asks_.begin()->second.get();
}

double MarketSnapshot::get_spread() const {
    auto* bid = get_best_bid();
    auto* ask = get_best_ask();
    if (!bid || !ask) return std::numeric_limits<double>::infinity();
    return ask->price - bid->price;
}
