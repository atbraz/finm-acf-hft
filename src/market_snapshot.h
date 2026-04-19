#pragma once

#include <functional>
#include <map>
#include <memory>

struct PriceLevel {
    double price;
    int quantity;
    PriceLevel(double p, int q) : price(p), quantity(q) {}
};

class MarketSnapshot {
    std::map<double, std::unique_ptr<PriceLevel>, std::greater<>> bids_;
    std::map<double, std::unique_ptr<PriceLevel>> asks_;

public:
    void update_bid(double price, int qty);
    void update_ask(double price, int qty);
    [[nodiscard]] const PriceLevel* get_best_bid() const;
    [[nodiscard]] const PriceLevel* get_best_ask() const;
    [[nodiscard]] double get_spread() const;
};
