#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "OrderBook.hpp"

namespace {

constexpr double kTick = 0.10;

double snap(double p) { return std::round(p / kTick) * kTick; }

}

int main(int argc, char** argv) {
    int           ops_per_sec = (argc > 1) ? std::atoi(argv[1]) : 2000;
    int           frame_hz    = (argc > 2) ? std::atoi(argv[2]) : 30;
    std::uint64_t seed        = (argc > 3) ? std::strtoull(argv[3], nullptr, 10) : 42ull;
    std::size_t   max_live    = (argc > 4) ? std::strtoull(argv[4], nullptr, 10) : 5000ull;

    OrderBook book;
    book.reserve(max_live * 2);

    std::mt19937                           rng(static_cast<unsigned>(seed));
    std::uniform_real_distribution<double> priceDist(95.0, 105.0);
    std::uniform_int_distribution<int>     qtyDist(1, 100);
    std::uniform_int_distribution<int>     sideDist(0, 1);
    std::uniform_int_distribution<int>     actionDist(0, 99);

    std::vector<std::string> live;
    live.reserve(max_live);
    std::uint64_t next_id = 0;

    const auto op_period    = std::chrono::nanoseconds(1'000'000'000LL / ops_per_sec);
    const auto frame_period = std::chrono::nanoseconds(1'000'000'000LL / frame_hz);
    auto       next_op_at   = std::chrono::steady_clock::now();
    auto       next_frame_at = next_op_at + frame_period;

    std::cout.sync_with_stdio(false);

    while (std::cout.good()) {
        auto now = std::chrono::steady_clock::now();
        if (now < next_op_at && now < next_frame_at) {
            std::this_thread::sleep_for(std::min(next_op_at, next_frame_at) - now);
            continue;
        }

        if (now >= next_op_at) {
            const int act = actionDist(rng);
            const bool can_modify_or_delete = !live.empty() && live.size() >= max_live / 4;

            if (!can_modify_or_delete || act < 70 || live.size() >= max_live) {
                if (live.size() < max_live) {
                    std::string id = "ORD" + std::to_string(next_id++);
                    double      p  = snap(priceDist(rng));
                    int         q  = qtyDist(rng);
                    bool        b  = sideDist(rng) == 1;
                    book.addOrder(id, p, q, b);
                    live.push_back(std::move(id));
                } else {
                    std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
                    std::size_t                                i = pick(rng);
                    book.deleteOrder(live[i]);
                    live[i] = std::move(live.back());
                    live.pop_back();
                }
            } else if (act < 85) {
                std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
                const std::string& id = live[pick(rng)];
                book.modifyOrder(id, snap(priceDist(rng)), qtyDist(rng));
            } else {
                std::uniform_int_distribution<std::size_t> pick(0, live.size() - 1);
                std::size_t                                i = pick(rng);
                book.deleteOrder(live[i]);
                live[i] = std::move(live.back());
                live.pop_back();
            }
            next_op_at += op_period;
        }

        if (now >= next_frame_at) {
            std::cout << "FRAME " << live.size() << "\n";
            for (const auto& [price, orders] : book.levels()) {
                int bid = 0, ask = 0;
                for (const auto& [id, o] : orders) {
                    (o.isBuy ? bid : ask) += o.quantity;
                }
                if (bid || ask) {
                    std::cout << "L " << price << " " << bid << " " << ask << "\n";
                }
            }
            std::cout << "END\n";
            std::cout.flush();
            next_frame_at += frame_period;
        }
    }
    return 0;
}
