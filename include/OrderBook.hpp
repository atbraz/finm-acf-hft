#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include "Config.hpp"
#include "Order.hpp"

class OrderBook {
public:
    OrderBook();
    void add(OrderT* order);                      // non-owning
    void cancel(std::uint64_t id);
    [[nodiscard]] const OrderT* best_bid() const;
    [[nodiscard]] const OrderT* best_ask() const;
    OrderT* pop_best_bid();
    OrderT* pop_best_ask();

private:
#if HFT_BOOK_IMPL_FLAT
    struct Level {
        double price;
        std::vector<OrderT*> orders;       // FIFO
    };
    // bids sorted descending, asks sorted ascending
    std::vector<Level> bids_;
    std::vector<Level> asks_;
    void insert_sorted(std::vector<Level>& side, OrderT* o, bool descending);
    bool erase_from(std::vector<Level>& side, std::uint64_t id);
#else
    std::multimap<double, OrderT*, std::greater<>> bids_;
    std::multimap<double, OrderT*>                 asks_;
#endif
};
