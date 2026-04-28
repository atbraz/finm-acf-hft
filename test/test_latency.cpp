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
