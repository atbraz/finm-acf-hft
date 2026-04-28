#pragma once

#include <cstdint>
#include <type_traits>

enum class OrderStatus : std::uint8_t {
    New,
    PartiallyFilled,
    Filled,
    Cancelled,
};

template <typename Price, typename OrderId>
struct Order {
    static_assert(std::is_arithmetic_v<Price>,
                  "Order price type must be arithmetic");
    static_assert(std::is_integral_v<OrderId>,
                  "Order ID type must be an integer");

    OrderId      id{};
    Price        price{};
    int          quantity{};
    int          filled{};
    bool         is_buy{};
    OrderStatus  status{OrderStatus::New};
};

using OrderT = Order<double, std::uint64_t>;
