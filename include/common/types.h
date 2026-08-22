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

#pragma pack(push, 1) 

struct ItchAddOrder {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6]; // 6-byte integer in ITCH 5.0
    std::uint64_t order_reference_number;
    char buy_sell_indicator;
    std::uint32_t shares;
    std::uint64_t stock;
    std::uint32_t price;
};

struct ItchOrderExecuted {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    std::uint32_t executed_shares;
    std::uint64_t match_number;
};

struct ItchOrderExecutedWithPrice {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    std::uint32_t executed_shares;
    std::uint64_t match_number;
    char printable;
    std::uint32_t execution_price;
};

struct ItchOrderCancel {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    std::uint32_t canceled_shares;
};

struct ItchOrderDelete {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
};

struct ItchOrderReplace {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t original_order_reference_number;
    std::uint64_t new_order_reference_number;
    std::uint32_t shares;
    std::uint32_t price;
};

#pragma pack(pop) // Restore default compiler alignment