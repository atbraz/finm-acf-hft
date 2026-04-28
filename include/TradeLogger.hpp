#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

struct Trade {
    std::uint64_t buy_id;
    std::uint64_t sell_id;
    double        price;
    int           qty;
    std::chrono::nanoseconds latency;
};

class TradeLogger {
public:
    explicit TradeLogger(const std::string& path);
    ~TradeLogger();

    TradeLogger(const TradeLogger&) = delete;
    TradeLogger& operator=(const TradeLogger&) = delete;

    void log(const Trade& t);
    void flush();

private:
    static constexpr std::size_t kBatchSize = 4096;
    std::vector<Trade> buf_;
    std::ofstream      out_;
};
