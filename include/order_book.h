#pragma once

#include "types.h"
#include <unordered_map>
#include <map>
#include <optional>


struct Order
{
    OrderId order_id;
    Price price;
    Quantity quantity;
    Side side;
};

class OrderBook
{
public:
    OrderBook() = default;

    void add_order(OrderId order_id, Side side, Quantity quantity, Price price);
    OrderStatus executed_order(OrderId order_id, Quantity quantity);
    OrderStatus executed_at_price_order(OrderId order_id, Quantity quantity, Price price);
    OrderStatus cancel_order(OrderId order_id, Quantity quantity);
    void delete_order(OrderId order_id);
    void replace_order(OrderId old_id, OrderId new_id, Quantity quantity, Price price);

    
    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;
    TotalQuantity bid_quantity(Price price) const;
    TotalQuantity ask_quantity(Price price) const;
    size_t bid_levels() const;
    size_t ask_levels() const;
    bool empty() const;

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
private:
    std::unordered_map<OrderId, Order> orders_;
    std::map<Price, TotalQuantity, std::greater<int64_t>> bids_;
    std::map<Price, TotalQuantity> asks_;
};