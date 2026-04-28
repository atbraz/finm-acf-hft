#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>
#include <random>
#include <algorithm>
#include "Timer.hpp"

TEST_CASE("Timer measures elapsed nanoseconds", "[timer]") {
    Timer t;
    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    long long ns = t.stop();
    REQUIRE(ns >= 1'000'000);
    REQUIRE(ns <  100'000'000);
}

TEST_CASE("ScopedTimer pushes one sample on dtor", "[timer]") {
    std::vector<long long> samples;
    samples.reserve(4);
    {
        ScopedTimer s(samples);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    REQUIRE(samples.size() == 1);
    REQUIRE(samples[0] >= 500'000);
}

#include "Order.hpp"

TEST_CASE("Order template instantiates with valid types", "[order]") {
    using OT = Order<double, std::uint64_t>;
    OT o{};
    o.id = 7;
    o.price = 100.5;
    o.quantity = 10;
    o.is_buy = true;
    o.status = OrderStatus::New;

    REQUIRE(o.id == 7);
    REQUIRE(o.price == 100.5);
    REQUIRE(o.is_buy);
    REQUIRE(o.status == OrderStatus::New);
    REQUIRE(o.filled == 0);
}

TEST_CASE("OrderT alias is Order<double, uint64_t>", "[order]") {
    STATIC_REQUIRE(std::is_same_v<OrderT, Order<double, std::uint64_t>>);
}

#include "ObjectPool.hpp"

TEST_CASE("ObjectPool round-trip allocation", "[pool]") {
    ObjectPool<OrderT, 4> pool;
    auto* a = pool.allocate();
    auto* b = pool.allocate();
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(a != b);
    pool.deallocate(a);
    pool.deallocate(b);
    auto* c = pool.allocate();
    REQUIRE((c == a || c == b));
    pool.deallocate(c);
}

TEST_CASE("ObjectPool returns nullptr on exhaustion", "[pool]") {
    ObjectPool<OrderT, 2> pool;
    auto* a = pool.allocate();
    auto* b = pool.allocate();
    auto* c = pool.allocate();
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(c == nullptr);
    pool.deallocate(a);
    pool.deallocate(b);
}

TEST_CASE("ObjectPool slots respect alignof(T)", "[pool]") {
    ObjectPool<OrderT, 8> pool;
    auto* a = pool.allocate();
    REQUIRE(reinterpret_cast<std::uintptr_t>(a) % alignof(OrderT) == 0);
    pool.deallocate(a);
}

TEST_CASE("PoolDeleter returns slot to pool", "[pool]") {
    ObjectPool<OrderT, 2> pool;
    {
        std::unique_ptr<OrderT, PoolDeleter<OrderT>> p(
            pool.allocate(), PoolDeleter<OrderT>{&pool});
        REQUIRE(p != nullptr);
    }
    auto* a = pool.allocate();
    auto* b = pool.allocate();
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    pool.deallocate(a);
    pool.deallocate(b);
}

#include "Config.hpp"

TEST_CASE("make_pool_shared returns the slot to the pool on last unref", "[config]") {
    hft::OrderPool pool;
    const auto initial = pool.available();
    {
        auto sp = hft::make_pool_shared(pool);
        sp->id = 42;
        sp->price = 99.5;
        REQUIRE(sp.use_count() == 1);
        REQUIRE(pool.available() == initial - 1);
    }
    REQUIRE(pool.available() == initial);
}

#include "MarketData.hpp"

TEST_CASE("MarketDataFeed::generate is deterministic by seed", "[feed]") {
    auto a = MarketDataFeed::generate(100, 42);
    auto b = MarketDataFeed::generate(100, 42);
    REQUIRE(a.size() == 100);
    REQUIRE(b.size() == 100);
    for (std::size_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i].bid_price == b[i].bid_price);
        REQUIRE(a[i].ask_price == b[i].ask_price);
        REQUIRE(a[i].bid_qty   == b[i].bid_qty);
        REQUIRE(a[i].ask_qty   == b[i].ask_qty);
    }
}

TEST_CASE("MarketDataFeed::generate produces a positive spread", "[feed]") {
    auto ticks = MarketDataFeed::generate(1000, 7);
    for (const auto& md : ticks) {
        REQUIRE(md.ask_price > md.bid_price);
        REQUIRE(md.bid_qty > 0);
        REQUIRE(md.ask_qty > 0);
    }
}

#include "OrderManager.hpp"

TEST_CASE("OrderManager creates orders with monotonic ids", "[oms]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    auto a = oms.create(100.0, 10, true);
    auto b = oms.create(101.0, 5,  false);
    REQUIRE(a);
    REQUIRE(b);
    REQUIRE(a->id == 1);
    REQUIRE(b->id == 2);
    REQUIRE(oms.active_count() == 2);
}

TEST_CASE("OrderManager partial then full fill", "[oms]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    auto o = oms.create(100.0, 10, true);
    oms.on_fill(o->id, 4);
    REQUIRE(o->status == OrderStatus::PartiallyFilled);
    REQUIRE(o->filled == 4);
    REQUIRE(oms.active_count() == 1);
    oms.on_fill(o->id, 6);
    REQUIRE(oms.active_count() == 0);
}

TEST_CASE("OrderManager cancellation removes order", "[oms]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    auto o = oms.create(100.0, 10, false);
    oms.on_cancel(o->id);
    REQUIRE(oms.active_count() == 0);
}

TEST_CASE("Order memory returns to pool on full fill", "[oms]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    const auto initial = pool.available();
    {
        auto o = oms.create(100.0, 1, true);
        REQUIRE(pool.available() == initial - 1);
        oms.on_fill(o->id, 1);
        REQUIRE(pool.available() == initial - 1);
    }
    REQUIRE(pool.available() == initial);
}

#include "OrderBook.hpp"

namespace {
OrderT make_order(std::uint64_t id, double price, int qty, bool is_buy) {
    OrderT o{};
    o.id = id;
    o.price = price;
    o.quantity = qty;
    o.is_buy = is_buy;
    return o;
}
}

TEST_CASE("OrderBook tracks best bid/ask after inserts", "[book]") {
    OrderBook book;
    OrderT b1 = make_order(1, 100.10, 10, true);
    OrderT b2 = make_order(2, 100.20, 10, true);
    OrderT a1 = make_order(3, 100.40, 10, false);
    OrderT a2 = make_order(4, 100.30, 10, false);
    book.add(&b1);
    book.add(&b2);
    book.add(&a1);
    book.add(&a2);

    REQUIRE(book.best_bid() != nullptr);
    REQUIRE(book.best_ask() != nullptr);
    REQUIRE(book.best_bid()->price == 100.20);
    REQUIRE(book.best_ask()->price == 100.30);
}

TEST_CASE("OrderBook cancel removes the matching id", "[book]") {
    OrderBook book;
    OrderT b1 = make_order(1, 100.10, 10, true);
    OrderT b2 = make_order(2, 100.20, 10, true);
    book.add(&b1);
    book.add(&b2);
    book.cancel(2);
    REQUIRE(book.best_bid()->price == 100.10);
}

TEST_CASE("OrderBook pop_best removes top of book", "[book]") {
    OrderBook book;
    OrderT a1 = make_order(1, 100.30, 10, false);
    OrderT a2 = make_order(2, 100.40, 10, false);
    book.add(&a1);
    book.add(&a2);
    OrderT* popped = book.pop_best_ask();
    REQUIRE(popped != nullptr);
    REQUIRE(popped->id == 1);
    REQUIRE(book.best_ask()->price == 100.40);
}

TEST_CASE("OrderBook returns null on empty side", "[book]") {
    OrderBook book;
    REQUIRE(book.best_bid() == nullptr);
    REQUIRE(book.best_ask() == nullptr);
    REQUIRE(book.pop_best_bid() == nullptr);
    REQUIRE(book.pop_best_ask() == nullptr);
}

#include <filesystem>
#include <fstream>
#include "TradeLogger.hpp"

TEST_CASE("TradeLogger flushes batched writes on dtor", "[logger]") {
    auto path = std::filesystem::temp_directory_path() / "hft_phase4_test_log.csv";
    std::filesystem::remove(path);
    {
        TradeLogger log(path.string());
        log.log(Trade{1, 2, 100.5, 10, std::chrono::nanoseconds(123)});
        log.log(Trade{3, 4, 100.6, 5,  std::chrono::nanoseconds(456)});
    }
    std::ifstream in(path);
    std::string line;
    int n = 0;
    while (std::getline(in, line)) ++n;
    REQUIRE(n >= 2);
    std::filesystem::remove(path);
}

#include "MatchingEngine.hpp"

TEST_CASE("MatchingEngine matches a crossing pair", "[engine]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    OrderBook    book;
    auto path = std::filesystem::temp_directory_path() / "hft_phase4_engine_log.csv";
    std::filesystem::remove(path);
    {
        TradeLogger log(path.string());
        MatchingEngine engine(book, oms, log);

        auto buy  = oms.create(100.50, 10, true);
        auto sell = oms.create(100.40, 10, false);
        book.add(buy.get());
        book.add(sell.get());

        std::vector<long long> latencies;
        latencies.reserve(8);
        MarketData md{};
        md.bid_price = 100.50;
        md.ask_price = 100.40;
        engine.on_tick(md, latencies);

        REQUIRE(latencies.size() == 1);
        REQUIRE(oms.active_count() == 0);
        REQUIRE(book.best_bid() == nullptr);
        REQUIRE(book.best_ask() == nullptr);
    }
    std::filesystem::remove(path);
}

TEST_CASE("MatchingEngine handles partial fill", "[engine]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    OrderBook    book;
    auto path = std::filesystem::temp_directory_path() / "hft_phase4_partial_log.csv";
    std::filesystem::remove(path);
    {
        TradeLogger log(path.string());
        MatchingEngine engine(book, oms, log);

        auto buy  = oms.create(100.50, 7, true);
        auto sell = oms.create(100.40, 10, false);
        book.add(buy.get());
        book.add(sell.get());

        std::vector<long long> latencies;
        MarketData md{};
        engine.on_tick(md, latencies);

        REQUIRE(latencies.size() == 1);
        REQUIRE(oms.active_count() == 1);
        REQUIRE(book.best_ask() != nullptr);
        REQUIRE(book.best_ask()->id == sell->id);
    }
    std::filesystem::remove(path);
}

TEST_CASE("MatchingEngine no-op when book does not cross", "[engine]") {
    hft::OrderPool pool;
    OrderManager oms(pool);
    OrderBook    book;
    auto path = std::filesystem::temp_directory_path() / "hft_phase4_nocross_log.csv";
    std::filesystem::remove(path);
    {
        TradeLogger log(path.string());
        MatchingEngine engine(book, oms, log);

        auto buy  = oms.create(100.30, 10, true);
        auto sell = oms.create(100.50, 10, false);
        book.add(buy.get());
        book.add(sell.get());

        std::vector<long long> latencies;
        MarketData md{};
        engine.on_tick(md, latencies);

        REQUIRE(latencies.size() == 1);
        REQUIRE(oms.active_count() == 2);
    }
    std::filesystem::remove(path);
}

namespace {
long long percentile(std::vector<long long>& v, double p) {
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    auto idx = static_cast<std::size_t>(static_cast<double>(v.size()) * p);
    if (idx >= v.size()) idx = v.size() - 1;
    return v[idx];
}
}

TEST_CASE("Latency smoke: 10k synthetic ticks under 50us p99", "[latency][.slow]") {
    auto ticks = MarketDataFeed::generate(10'000, 42);
    hft::OrderPool pool;
    OrderManager oms(pool);
    OrderBook    book;
    auto path = std::filesystem::temp_directory_path() / "hft_phase4_smoke.csv";
    std::filesystem::remove(path);
    TradeLogger log(path.string());
    MatchingEngine engine(book, oms, log);

    std::vector<long long> latencies;
    latencies.reserve(ticks.size());

    std::mt19937_64 rng(123);  // NOLINT(cert-msc32-c,cert-msc51-cpp)
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

    REQUIRE(latencies.size() == ticks.size());
    auto p99 = percentile(latencies, 0.99);
    INFO("p99=" << p99 << " ns");
    REQUIRE(p99 < 50'000);
    std::filesystem::remove(path);
}
