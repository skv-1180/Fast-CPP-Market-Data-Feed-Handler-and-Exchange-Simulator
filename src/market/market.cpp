#include "market/market.h"
#include "common/utility.h"
#include <iostream>

Market::Market(std::size_t max_orders) 
    :global_orders_(max_orders) 
{
}
    
void Market::add_order(OrderId order_id, Side side, Quantity qty, SymbolId symbol_id, Price price)
{
    if (order_id >= global_orders_.size()) [[unlikely]] 
    {
        global_orders_.resize(order_id * 2);
    }

    global_orders_[order_id] = Order{order_id, price, qty, side, symbol_id, true};

    OrderBook& book = books_[symbol_id];

    book.add_level_quantity(price, qty, side);
}

void Market::reduce_order_size(OrderId order_id, Quantity qty)
{
    Order& order = global_orders_[order_id];
    if (!order.active) [[unlikely]] return;

    OrderBook& book = books_[order.symbol_id];

    if (qty >= order.quantity)
    {
        book.reduce_level_quantity(order.price, order.quantity, order.side);
        order.active = false; 
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
    Order& order = global_orders_[order_id];
    if (!order.active) [[unlikely]] return;

    OrderBook& book = books_[order.symbol_id];

    book.reduce_level_quantity(order.price, order.quantity, order.side);
    order.active = false;
}

void Market::replace_order(OrderId oldId, OrderId newId, Quantity qty, Price price)
{
    Order& old_order = global_orders_[oldId];
    if (!old_order.active) [[unlikely]] return;

    const Order old_order_copy = old_order;
    OrderBook& book = books_[old_order_copy.symbol_id];

    book.reduce_level_quantity(old_order_copy.price, old_order_copy.quantity, old_order_copy.side);
    old_order.active = false;

    if (newId >= global_orders_.size()) [[unlikely]] {
        global_orders_.resize(newId * 2);
    }

    global_orders_[newId] = Order{newId, price, qty, old_order_copy.side, old_order_copy.symbol_id, true};

    book.add_level_quantity(price, qty, old_order_copy.side);
}

const OrderBook* Market::find_book(SymbolId symbol_id) const
{
    if (symbol_id >= books_.size() || books_[symbol_id].empty()) return nullptr;
    return &books_[symbol_id];
}

void Market::print_best_bid_ask() const
{
    for (std::size_t symbol_id = 0; symbol_id < books_.size(); ++symbol_id)
    {
        const auto& book = books_[symbol_id];
        
        if (book.empty()) continue;
        
        auto bid = book.best_bid();
        auto ask = book.best_ask();
        
        std::cout << "Symbol ID: " << symbol_id << '\n';

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
        
        std::cout << "Bid Levels: " << book.bid_levels() << '\n';
        std::cout << "Ask Levels: " << book.ask_levels() << '\n';
    }
}


