#pragma once

#include <vector>
#include "MarketData.hpp"
#include "OrderBook.hpp"
#include "OrderManager.hpp"
#include "TradeLogger.hpp"

class MatchingEngine {
public:
    MatchingEngine(OrderBook& book, OrderManager& oms, TradeLogger& log);
    void on_tick(const MarketData& md, std::vector<long long>& latencies);

private:
    OrderBook&    book_;
    OrderManager& oms_;
    TradeLogger&  log_;
};
