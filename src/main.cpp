#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include "Config.hpp"
#include "MarketData.hpp"
#include "MatchingEngine.hpp"
#include "OrderBook.hpp"
#include "OrderManager.hpp"
#include "TradeLogger.hpp"

namespace {
struct Stats {
    long long min, max, p50, p95, p99;
    double mean, stddev;
};

Stats summarize(std::vector<long long> v) {
    std::sort(v.begin(), v.end());
    const auto n = v.size();
    auto pct = [&](double p) {
        auto i = static_cast<std::size_t>(static_cast<double>(n) * p);
        return v[std::min(i, n - 1)];
    };
    double mean = std::accumulate(v.begin(), v.end(), 0.0) / static_cast<double>(n);
    double var  = 0.0;
    for (auto x : v) {
        double d = static_cast<double>(x) - mean;
        var += d * d;
    }
    return Stats{
        v.front(), v.back(),
        pct(0.50), pct(0.95), pct(0.99),
        mean,
        std::sqrt(var / static_cast<double>(n))
    };
}
}  // namespace

int main(int argc, char** argv) {
    const std::size_t n_ticks = (argc > 1) ? std::stoul(argv[1]) : 10'000;
    const std::uint64_t seed   = (argc > 2) ? std::stoull(argv[2]) : 42;

    auto ticks = MarketDataFeed::generate(n_ticks, seed);

    hft::OrderPool pool;
    OrderManager   oms(pool);
    OrderBook      book;
    TradeLogger    log("results/trades.csv");
    MatchingEngine engine(book, oms, log);

    std::vector<long long> latencies;
    latencies.reserve(n_ticks);

    std::mt19937_64 rng(seed);
    for (const auto& md : ticks) {
        auto bid = oms.create(md.bid_price, 10, true);
        auto ask = oms.create(md.ask_price, 10, false);
        if (bid) book.add(bid.get());
        if (ask) book.add(ask.get());
        if (rng() % 10 == 0) {
            auto cross = oms.create(md.ask_price, 10, true);
            if (cross) book.add(cross.get());
        }
        engine.on_tick(md, latencies);
    }

    auto s = summarize(latencies);
    std::cout << std::fixed << std::setprecision(2)
              << "Tick-to-Trade Latency (ns) over " << n_ticks << " ticks\n"
              << "  Min:    " << s.min    << "\n"
              << "  Mean:   " << s.mean   << "\n"
              << "  Stddev: " << s.stddev << "\n"
              << "  p50:    " << s.p50    << "\n"
              << "  p95:    " << s.p95    << "\n"
              << "  p99:    " << s.p99    << "\n"
              << "  Max:    " << s.max    << "\n";

    return 0;
}
