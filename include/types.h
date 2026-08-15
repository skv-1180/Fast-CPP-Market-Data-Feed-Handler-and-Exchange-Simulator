#pragma once

#include <cstdint>

using OrderId = uint64_t;
using Quantity = uint32_t;
using Price = uint32_t;
using Symbol = uint64_t;
using TotalQuantity = uint64_t;

enum class Side {
    Buy, Sell
};

struct BestPrice {
    Price bid = 0;
    Quantity bidQty = 0;
    Price ask = 0;
    Quantity askQty = 0;
};