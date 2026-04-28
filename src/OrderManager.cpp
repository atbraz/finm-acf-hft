#include "OrderManager.hpp"

OrderManager::OrderManager(hft::OrderPool& pool) : pool_(pool) {}

std::shared_ptr<OrderT> OrderManager::create(double price, int quantity, bool is_buy) {
    auto sp = hft::make_pool_shared(pool_);
    if (!sp) return nullptr;
    sp->id       = next_id_++;
    sp->price    = price;
    sp->quantity = quantity;
    sp->filled   = 0;
    sp->is_buy   = is_buy;
    sp->status   = OrderStatus::New;
    live_.emplace(sp->id, sp);
    return sp;
}

void OrderManager::on_fill(std::uint64_t id, int qty) {
    auto it = live_.find(id);
    if (it == live_.end()) return;
    auto& o = *it->second;
    o.filled += qty;
    if (o.filled >= o.quantity) {
        o.status = OrderStatus::Filled;
        live_.erase(it);
    } else {
        o.status = OrderStatus::PartiallyFilled;
    }
}

void OrderManager::on_cancel(std::uint64_t id) {
    auto it = live_.find(id);
    if (it == live_.end()) return;
    it->second->status = OrderStatus::Cancelled;
    live_.erase(it);
}

std::size_t OrderManager::active_count() const noexcept { return live_.size(); }

std::shared_ptr<OrderT> OrderManager::get(std::uint64_t id) const {
    auto it = live_.find(id);
    return it == live_.end() ? nullptr : it->second;
}
