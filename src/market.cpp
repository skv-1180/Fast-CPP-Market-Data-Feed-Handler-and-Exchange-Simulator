#include "market.h"

void Market::add_order(OrderId order_id, Side side, Quantity qty,
                       Symbol symbol, Price price)
{
    auto [it, _] = books_.try_emplace(symbol, OrderBook{});
    OrderBook& book = it->second;
    book.add_order(order_id, side, qty, price);
    order_index_[order_id] = &book;
}

void Market::executed_order(OrderId order_id, Quantity qty)
{
    auto it = order_index_.find(order_id);
    if (it == order_index_.end()) return;

    auto order_status = it->second->executed_order(order_id, qty);

    if (order_status == OrderStatus::Removed)
        order_index_.erase(it);
}

void Market::executed_at_price_order(OrderId order_id, Quantity qty, Price price)
{
    auto it = order_index_.find(order_id);
    if (it == order_index_.end()) return;

    auto order_status = it->second->executed_at_price_order(order_id, qty, price);

    if (order_status == OrderStatus::Removed)
        order_index_.erase(it);
}

void Market::cancel_order(OrderId order_id, Quantity qty)
{
    auto it = order_index_.find(order_id);
    if (it == order_index_.end()) return;

    auto order_status = it->second->cancel_order(order_id, qty);

    if (order_status == OrderStatus::Removed)
        order_index_.erase(it);
}

void Market::delete_order(OrderId order_id)
{
    auto it = order_index_.find(order_id);
    if (it == order_index_.end()) return;

    it->second->delete_order(order_id);
    order_index_.erase(it);
}

void Market::replace_order(OrderId oldId, OrderId newId, Quantity qty, Price price)
{
    auto it = order_index_.find(oldId);
    if (it == order_index_.end()) return;

    OrderBook* book = it->second;

    book->replace_order(oldId, newId, qty, price);

    order_index_.erase(it);
    order_index_[newId] = book;
}