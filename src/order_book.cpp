#include "order_book.h"

void OrderBook::add_order(OrderId order_id, Side side, Quantity quantity, Price price)
{
    Order order{order_id, price, quantity, side};
    orders_[order_id] = order;

    if (side == Side::Buy)
    {
        bids_[price] += quantity;
    }
    else
    {
        asks_[price] += quantity;
    }
}

OrderStatus OrderBook::executed_order(OrderId order_id, Quantity quantity)
{
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return OrderStatus::Removed;   

    Order& order = it->second;

    if (quantity >= order.quantity)
    {
        delete_order(order_id);
        return OrderStatus::Removed;
    }

    order.quantity -= quantity;
    if (order.side == Side::Buy)
    {
        bids_[order.price] -= quantity;
    }
    else
    {
        asks_[order.price] -= quantity;
    }
    
    return OrderStatus::Alive;
}

OrderStatus OrderBook::executed_at_price_order(OrderId order_id, Quantity quantity, Price price)
{
    return executed_order(order_id, quantity);
}

OrderStatus OrderBook::cancel_order(OrderId order_id, Quantity quantity)
{
    return executed_order(order_id, quantity);
}

void OrderBook::delete_order(OrderId order_id)
{
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return;

    const Order& order = it->second;
    if (order.side == Side::Buy)
    {
        auto level = bids_.find(order.price);
        level->second -= order.quantity;

        if (level->second == 0)
            bids_.erase(level);
    }
    else
    {
        auto level = asks_.find(order.price);
        level->second -= order.quantity;

        if (level->second == 0)
            asks_.erase(level);
    }
    orders_.erase(it);
}

void OrderBook::replace_order(OrderId old_id, OrderId new_id, Quantity quantity, Price price)
{
    auto it = orders_.find(old_id);
    if (it == orders_.end()) return;

    Side side = it->second.side;
    delete_order(old_id);
    add_order(new_id, side, quantity, price);
}

std::optional<Price> OrderBook::best_bid() const
{
    if (bids_.empty())
        return std::nullopt;

    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const
{
    if (asks_.empty())
        return std::nullopt;

    return asks_.begin()->first;
}

TotalQuantity OrderBook::bid_quantity(Price price) const
{
    auto it = bids_.find(price);
    return (it != bids_.end()) ? it->second : TotalQuantity{0};
}

TotalQuantity OrderBook::ask_quantity(Price price) const
{
    auto it = asks_.find(price);
    return (it != asks_.end()) ? it->second : TotalQuantity{0};
}

size_t OrderBook::bid_levels() const
{
    return bids_.size();
}

size_t OrderBook::ask_levels() const
{
    return asks_.size();
}

bool OrderBook::empty() const
{
    return orders_.empty();
}