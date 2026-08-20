#pragma once

#include <cstdint>

using OrderId = std::uint64_t;
using Quantity = std::uint32_t;
using Price = std::uint32_t;
using Symbol = std::uint64_t;
using TotalQuantity = std::uint64_t;

enum class Side {
    Buy, Sell
};

struct BestPrice {
    Price bid = 0;
    Quantity bidQty = 0;
    Price ask = 0;
    Quantity askQty = 0;
};

enum class OrderStatus {
    Alive,
    Removed
};