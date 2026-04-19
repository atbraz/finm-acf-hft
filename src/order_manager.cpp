#include "order_manager.h"

#include <iomanip>
#include <iostream>

int OrderManager::place_order(Side side, double price, int qty) {
    int id = next_id_++;
    auto order = std::make_unique<MyOrder>();
    order->id = id;
    order->side = side;
    order->price = price;
    order->quantity = qty;
    orders_.emplace(id, std::move(order));
    return id;
}

void OrderManager::cancel(int id) {
    auto it = orders_.find(id);
    if (it == orders_.end()) {
        std::cerr << "[Order] Unknown order ID: " << id << "\n";
        return;
    }
    it->second->status = OrderStatus::Cancelled;
    std::cout << "[Order] Order " << id << " cancelled and removed\n";
    orders_.erase(it);
}

void OrderManager::handle_fill(int id, int filled_qty) {
    auto it = orders_.find(id);
    if (it == orders_.end()) {
        std::cerr << "[Order] Fill for unknown order ID: " << id << "\n";
        return;
    }

    auto& order = it->second;
    order->filled += filled_qty;

    if (order->filled >= order->quantity) {
        order->status = OrderStatus::Filled;
        std::cout << "[Order] Order " << id << " completed ("
                  << order->quantity << " / " << order->quantity
                  << ") and removed\n";
        orders_.erase(it);
    } else {
        order->status = OrderStatus::PartiallyFilled;
        std::cout << "[Order] Order " << id << " partially filled: "
                  << order->filled << " / " << order->quantity << "\n";
    }
}

void OrderManager::print_active_orders() const {
    if (orders_.empty()) {
        std::cout << "[Order] No active orders\n";
        return;
    }
    for (const auto& [id, order] : orders_) {
        std::cout << "[Order] ID " << id << " "
                  << (order->side == Side::Buy ? "BUY" : "SELL")
                  << " " << std::fixed << std::setprecision(2) << order->price
                  << " x " << order->quantity
                  << " (filled: " << order->filled << ")\n";
    }
}
