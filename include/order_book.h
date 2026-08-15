#pragma once

#include "types.h"
#include <unordered_map>
#include <map>

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
    explicit OrderBook(){};

    void add_order(OrderId order_id, Side side, Quantity quantity, Price price);
    void executed_order(OrderId order_id, Quantity quantity);
    void executed_at_price_order(OrderId order_id, Quantity quantity, Price price);
    void cancel_order(OrderId order_id, Quantity quantity);
    void delete_order(OrderId order_id);
    void replace_order(OrderId original_order_id, OrderId new_order_id, Quantity quantity, Price price);
private:
    std::unordered_map<OrderId, Order> orders_;
    std::map<Price, TotalQuantity, std::greater<int64_t>> bids_;
    std::map<Price, TotalQuantity> asks_;
};