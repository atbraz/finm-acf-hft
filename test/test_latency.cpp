#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>
#include "Timer.hpp"

TEST_CASE("Timer measures elapsed nanoseconds", "[timer]") {
    Timer t;
    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    long long ns = t.stop();
    REQUIRE(ns >= 1'000'000);                  // at least 1 ms
    REQUIRE(ns <  100'000'000);                // and well under 100 ms
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
    auto* c = pool.allocate();      // should reuse a slot
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
