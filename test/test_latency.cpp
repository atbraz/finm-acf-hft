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
