#include "market/market.h"
#include "common/utility.h"
#include <iostream>

void Market::add_order(OrderId order_id, Side side, Quantity qty,
                       Symbol symbol, Price price)
{
    global_orders_[order_id] = Order{order_id, price, qty, side, symbol};

    auto [it, _] = books_.try_emplace(symbol);
    OrderBook& book = it->second;

    book.add_level_quantity(price, qty, side);
}

void Market::reduce_order_size(OrderId order_id, Quantity qty)
{
    auto it = global_orders_.find(order_id);
    if (it == global_orders_.end()) return;

    Order& order = it->second;
    OrderBook& book = books_[order.symbol];

    if (qty >= order.quantity)
    {
        book.reduce_level_quantity(order.price, order.quantity, order.side);
        global_orders_.erase(it);
    }
    else
    {
        book.reduce_level_quantity(order.price, qty, order.side);
        order.quantity -= qty;
    }
}

void Market::executed_order(OrderId order_id, Quantity qty)
{
    reduce_order_size(order_id, qty);
}

void Market::executed_at_price_order(OrderId order_id, Quantity qty, [[maybe_unused]] Price price)
{
    reduce_order_size(order_id, qty); 
}

void Market::cancel_order(OrderId order_id, Quantity qty)
{
    reduce_order_size(order_id, qty);
}

void Market::delete_order(OrderId order_id)
{
    auto it = global_orders_.find(order_id);
    if (it == global_orders_.end()) return;

    const Order& order = it->second;
    OrderBook& book = books_[order.symbol];

    book.reduce_level_quantity(order.price, order.quantity, order.side);
    global_orders_.erase(it);
}

void Market::replace_order(OrderId oldId, OrderId newId, Quantity qty, Price price)
{
    auto it = global_orders_.find(oldId);
    if (it == global_orders_.end()) return;

    const Order old_order = it->second;
    OrderBook& book = books_[old_order.symbol];

    book.reduce_level_quantity(old_order.price, old_order.quantity, old_order.side);
    global_orders_.erase(it);

    global_orders_[newId] = Order{newId, price, qty, old_order.side, old_order.symbol};

    book.add_level_quantity(price, qty, old_order.side);
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
