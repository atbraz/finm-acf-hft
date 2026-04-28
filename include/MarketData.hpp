#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include "Config.hpp"

#if HFT_ALIGN_CACHE
struct alignas(64) MarketData {
#else
struct MarketData {
#endif
    std::string symbol;
    double bid_price{};
    double ask_price{};
    int    bid_qty{};
    int    ask_qty{};
    std::chrono::high_resolution_clock::time_point timestamp{};
};

class MarketDataFeed {
public:
    static std::vector<MarketData> generate(std::size_t n, std::uint64_t seed);
    static std::vector<MarketData> load(const std::string& path);
};
