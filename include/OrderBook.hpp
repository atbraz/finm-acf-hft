#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include "Order.hpp"

class OrderBook {
public:
    void addOrder(const std::string& id, double price, int quantity, bool isBuy);
    void modifyOrder(const std::string& id, double newPrice, int newQuantity);
    void deleteOrder(const std::string& id);

    [[nodiscard]] bool contains(const std::string& id) const;
    [[nodiscard]] std::size_t size() const;
    void reserve(std::size_t n);

    using LevelMap = std::map<double, std::unordered_map<std::string, Order>>;
    [[nodiscard]] const LevelMap& levels() const noexcept { return orderLevels; }

private:
    LevelMap                               orderLevels;
    std::unordered_map<std::string, Order> orderLookup;
};
