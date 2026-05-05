#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "OrderBook.hpp"

namespace {
struct Input {
    std::string id;
    double      price;
    int         qty;
    bool        isBuy;
};
}

int main(int argc, char** argv) {
    int           n       = (argc > 1) ? std::atoi(argv[1]) : 10000;
    std::uint64_t seed    = (argc > 2) ? std::strtoull(argv[2], nullptr, 10) : 42ull;
    bool          reserve = (argc > 3 && std::string(argv[3]) == "reserve");

    std::mt19937                           rng(static_cast<unsigned>(seed));
    std::uniform_real_distribution<double> priceDist(50.0, 100.0);
    std::uniform_int_distribution<int>     qtyDist(1, 500);
    std::uniform_int_distribution<int>     sideDist(0, 1);

    std::vector<Input> in;
    in.reserve(n);
    for (int i = 0; i < n; ++i) {
        in.push_back({"ORD" + std::to_string(i),
                      priceDist(rng),
                      qtyDist(rng),
                      sideDist(rng) == 1});
    }

    OrderBook book;
    if (reserve) book.reserve(static_cast<std::size_t>(n));
    auto t0 = std::chrono::high_resolution_clock::now();
    for (const auto& o : in) book.addOrder(o.id, o.price, o.qty, o.isBuy);
    auto t1 = std::chrono::high_resolution_clock::now();

    auto              ns    = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    const char* const label = reserve ? "reserved" : "baseline";
    std::cout << label << "," << n << "," << seed << "," << ns << ","
              << static_cast<double>(ns) / n << "\n";
    return 0;
}
