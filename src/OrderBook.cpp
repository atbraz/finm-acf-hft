#include "OrderBook.hpp"

void OrderBook::addOrder(const std::string& id, double price, int quantity, bool isBuy) {
    Order o{id, price, quantity, isBuy};
    orderLevels[price][id] = o;
    orderLookup[id]        = o;
}

void OrderBook::modifyOrder(const std::string& id, double newPrice, int newQuantity) {
    auto it = orderLookup.find(id);
    if (it == orderLookup.end()) return;

    const Order old = it->second;
    auto level = orderLevels.find(old.price);
    if (level != orderLevels.end()) {
        level->second.erase(id);
        if (level->second.empty()) orderLevels.erase(level);
    }
    addOrder(id, newPrice, newQuantity, old.isBuy);
}

void OrderBook::deleteOrder(const std::string& id) {
    auto it = orderLookup.find(id);
    if (it == orderLookup.end()) return;

    auto level = orderLevels.find(it->second.price);
    if (level != orderLevels.end()) {
        level->second.erase(id);
        if (level->second.empty()) orderLevels.erase(level);
    }
    orderLookup.erase(it);
}

bool OrderBook::contains(const std::string& id) const {
    return orderLookup.find(id) != orderLookup.end();
}

std::size_t OrderBook::size() const {
    return orderLookup.size();
}
