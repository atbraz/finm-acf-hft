#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Config.hpp"
#include "Order.hpp"

class OrderManager {
public:
    explicit OrderManager(hft::OrderPool& pool);

    std::shared_ptr<OrderT> create(double price, int quantity, bool is_buy);
    void on_fill(std::uint64_t id, int qty);
    void on_cancel(std::uint64_t id);
    [[nodiscard]] std::size_t active_count() const noexcept;
    [[nodiscard]] std::shared_ptr<OrderT> get(std::uint64_t id) const;

private:
    hft::OrderPool& pool_;
    std::unordered_map<std::uint64_t, std::shared_ptr<OrderT>> live_;
    std::uint64_t next_id_{1};
};
