#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "Config.hpp"
#include "MarketData.hpp"
#include "MatchingEngine.hpp"
#include "OrderBook.hpp"
#include "OrderManager.hpp"
#include "TradeLogger.hpp"

namespace {
struct Args {
    std::size_t   ticks = 10'000;
    std::uint64_t seed  = 42;
    std::string   label = "unlabeled";
    std::string   out   = "results/bench.csv";
};

Args parse(int argc, char** argv) {
    Args a;
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string k = argv[i];
        std::string v = argv[i + 1];
        if      (k == "--ticks") a.ticks = std::stoul(v);
        else if (k == "--seed")  a.seed  = std::stoull(v);
        else if (k == "--label") a.label = v;
        else if (k == "--out")   a.out   = v;
    }
    return a;
}

long long pct(std::vector<long long>& v, double p) {
    auto i = static_cast<std::size_t>(static_cast<double>(v.size()) * p);
    return v[std::min(i, v.size() - 1)];
}
}  // namespace

int main(int argc, char** argv) {
    Args args = parse(argc, argv);

    auto ticks = MarketDataFeed::generate(args.ticks, args.seed);

    hft::OrderPool pool;
    OrderManager   oms(pool);
    OrderBook      book;
    TradeLogger    log("results/bench_trades.csv");
    MatchingEngine engine(book, oms, log);

    std::vector<long long> latencies;
    latencies.reserve(args.ticks);

    std::mt19937_64 rng(args.seed);
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

    std::sort(latencies.begin(), latencies.end());
    auto min  = latencies.front();
    auto max  = latencies.back();
    auto p50  = pct(latencies, 0.50);
    auto p95  = pct(latencies, 0.95);
    auto p99  = pct(latencies, 0.99);
    double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0)
                  / static_cast<double>(latencies.size());
    double var = 0.0;
    for (auto x : latencies) {
        double d = static_cast<double>(x) - mean;
        var += d * d;
    }
    double stddev = std::sqrt(var / static_cast<double>(latencies.size()));

    bool need_header = false;
    {
        std::ifstream in(args.out);
        need_header = !in.good() || in.peek() == std::ifstream::traits_type::eof();
    }
    std::ofstream out(args.out, std::ios::app);
    if (need_header) {
        out << "label,ticks,seed,raw_ptr,align_cache,use_pool,book_flat,"
            << "min_ns,mean_ns,stddev_ns,p50_ns,p95_ns,p99_ns,max_ns\n";
    }
    out << args.label << ',' << args.ticks << ',' << args.seed << ','
        << HFT_USE_RAW_PTR  << ',' << HFT_ALIGN_CACHE << ','
        << HFT_USE_POOL     << ',' << HFT_BOOK_IMPL_FLAT << ','
        << min << ',' << mean << ',' << stddev << ','
        << p50 << ',' << p95 << ',' << p99 << ',' << max << '\n';

    std::printf("[%s] mean=%.0fns p99=%lldns\n",
                args.label.c_str(), mean, p99);
    return 0;
}
