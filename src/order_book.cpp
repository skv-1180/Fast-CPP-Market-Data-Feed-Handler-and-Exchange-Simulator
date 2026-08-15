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

void OrderBook::executed_order(OrderId order_id, Quantity quantity)
{
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return;

    Order& order = it->second;

    if (quantity >= order.quantity)
    {
        delete_order(order_id);
        return;
    }

    order.quantity -= quantity;

    if (order.side == Side::Buy)
    {
        auto level = bids_.find(order.price);
        level->second -= quantity;

        if (level->second == 0)
            bids_.erase(level);
    }
    else
    {
        auto level = asks_.find(order.price);
        level->second -= quantity;

        if (level->second == 0)
            asks_.erase(level);
    }
}

void OrderBook::executed_at_price_order(OrderId order_id, Quantity quantity, Price price)
{
    executed_order(order_id, quantity);
}

void OrderBook::cancel_order(OrderId order_id, Quantity quantity)
{
    executed_order(order_id, quantity);
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

void OrderBook::replace_order(OrderId original_order_id, OrderId new_order_id, Quantity quantity, Price price)
{
    auto it = orders_.find(original_order_id);
    if (it == orders_.end()) return;

    Side side = it->second.side;
    delete_order(original_order_id);
    add_order(new_order_id, side, quantity, price);
}

