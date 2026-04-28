#include "TradeLogger.hpp"

TradeLogger::TradeLogger(const std::string& path) : out_(path) {
    buf_.reserve(kBatchSize);
    out_ << "buy_id,sell_id,price,qty,latency_ns\n";
}

TradeLogger::~TradeLogger() { flush(); }

void TradeLogger::log(const Trade& t) {
    buf_.push_back(t);
    if (buf_.size() >= kBatchSize) flush();
}

void TradeLogger::flush() {
    if (!out_.is_open()) return;
    for (const auto& t : buf_) {
        out_ << t.buy_id << ',' << t.sell_id << ','
             << t.price  << ',' << t.qty     << ','
             << t.latency.count() << '\n';
    }
    buf_.clear();
    out_.flush();
}
