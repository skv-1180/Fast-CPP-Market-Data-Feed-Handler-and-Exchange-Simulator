#include "market/market.h"
#include "market/utility.h"
#include <iostream>

void Market::add_order(OrderId order_id, Side side, Quantity qty,
                       Symbol symbol, Price price)
{
    auto [it, _] = books_.try_emplace(symbol);
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

const OrderBook* Market::find_book(Symbol symbol) const
{
    auto it = books_.find(symbol);
    return (it != books_.end()) ? &it->second : nullptr;
}


void Market::print_best_bid_ask() const
{
    using namespace md;
    for (const auto& it: books_){
        Symbol symbol = it.first;
        const auto& book = it.second;
        
        auto bid = book.best_bid();
        auto ask = book.best_ask();
        
        std::cout << "Symbol: " << symbol_to_string(symbol) << '\n';

        if (bid)
            std::cout << "Best bid: " << *bid
                        << " Qty: " << book.bid_quantity(*bid) << '\n';
        else
            std::cout << "Best bid: none\n";

        if (ask)
            std::cout << "Best ask: " << *ask
                        << " Qty: " << book.ask_quantity(*ask) << '\n';
        else
            std::cout << "Best ask: none\n";
        
        std::cout << "Bid Levels: " <<  book.bid_levels() << '\n';
        std::cout << "Ask Levels: " << book.ask_levels() << '\n';
    }
}