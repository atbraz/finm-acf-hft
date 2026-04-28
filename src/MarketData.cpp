#include "MarketData.hpp"

#include <fstream>
#include <random>
#include <sstream>

std::vector<MarketData> MarketDataFeed::generate(std::size_t n, std::uint64_t seed) {
    std::vector<MarketData> out;
    out.reserve(n);

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> step(0.0, 0.02);
    std::uniform_int_distribution<int> qty(50, 500);

    double mid = 100.0;
    constexpr double half_spread = 0.05;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        mid += step(rng);
        MarketData md;
        md.symbol     = "SYNTH";
        md.bid_price  = mid - half_spread;
        md.ask_price  = mid + half_spread;
        md.bid_qty    = qty(rng);
        md.ask_qty    = qty(rng);
        md.timestamp  = t0 + std::chrono::nanoseconds(i);
        out.push_back(std::move(md));
    }
    return out;
}

std::vector<MarketData> MarketDataFeed::load(const std::string& path) {
    std::vector<MarketData> out;
    std::ifstream f(path);
    if (!f.is_open()) return out;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        MarketData md;
        if (iss >> md.symbol >> md.bid_price >> md.ask_price
                >> md.bid_qty   >> md.ask_qty) {
            md.timestamp = std::chrono::high_resolution_clock::now();
            out.push_back(std::move(md));
        }
    }
    return out;
}
