#include "MatchingEngine.hpp"

#include "Timer.hpp"

MatchingEngine::MatchingEngine(OrderBook& book, OrderManager& oms, TradeLogger& log)
    : book_(book), oms_(oms), log_(log) {}

void MatchingEngine::on_tick(const MarketData&, std::vector<long long>& latencies) {
    ScopedTimer t(latencies);

    while (true) {
        const OrderT* bid = book_.best_bid();
        const OrderT* ask = book_.best_ask();
        if (!bid || !ask) [[likely]] break;
        if (bid->price < ask->price) [[likely]] break;

        OrderT* b = book_.pop_best_bid();
        OrderT* a = book_.pop_best_ask();

        const int b_remaining = b->quantity - b->filled;
        const int a_remaining = a->quantity - a->filled;
        const int qty         = b_remaining < a_remaining ? b_remaining : a_remaining;
        const double price    = a->price;

        oms_.on_fill(b->id, qty);
        oms_.on_fill(a->id, qty);

        log_.log(Trade{b->id, a->id, price, qty, std::chrono::nanoseconds(0)});

        if (b->filled < b->quantity) book_.add(b);
        if (a->filled < a->quantity) book_.add(a);
    }
}
