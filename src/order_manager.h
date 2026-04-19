#pragma once

#include <map>
#include <memory>
#include <iostream>

enum class Side { Buy, Sell };
enum class OrderStatus { New, PartiallyFilled, Filled, Cancelled };

struct MyOrder {
    int id;
    Side side;
    double price;
    int quantity;
    int filled = 0;
    OrderStatus status = OrderStatus::New;
};

class OrderManager {
    std::map<int, std::unique_ptr<MyOrder>> orders_;
    int next_id_ = 1;

public:
    int place_order(Side side, double price, int qty);
    void cancel(int id);
    void handle_fill(int id, int filled_qty);
    void print_active_orders() const;
};
