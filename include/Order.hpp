#pragma once

#include <string>

struct Order {
    std::string id;
    double      price;
    int         quantity;
    bool        isBuy;
};
