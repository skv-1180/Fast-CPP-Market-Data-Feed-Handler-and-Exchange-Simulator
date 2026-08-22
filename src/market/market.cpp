#include "market/market.h"
#include "common/utility.h"
#include <iostream>

Market::Market(std::size_t max_orders) {
    global_orders_.resize(max_orders);
}

void Market::add_order(OrderId order_id, Side side, Quantity qty,
                       Symbol symbol, Price price)
{
    if (order_id >= global_orders_.size()) [[unlikely]] 
    {
        global_orders_.resize(order_id * 2);
    }

    global_orders_[order_id] = Order{order_id, price, qty, side, symbol, true};

    auto [it, _] = books_.try_emplace(symbol);
    OrderBook& book = it->second;

    book.add_level_quantity(price, qty, side);
}

void Market::reduce_order_size(OrderId order_id, Quantity qty)
{
    Order& order = global_orders_[order_id];
    if (!order.active) [[unlikely]] return;

    OrderBook& book = books_[order.symbol];

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

    OrderBook& book = books_[order.symbol];

    book.reduce_level_quantity(order.price, order.quantity, order.side);
    order.active = false;
}

void Market::replace_order(OrderId oldId, OrderId newId, Quantity qty, Price price)
{
    Order& old_order = global_orders_[oldId];
    if (!old_order.active) [[unlikely]] return;

    const Order old_order_copy = old_order;
    OrderBook& book = books_[old_order_copy.symbol];

    book.reduce_level_quantity(old_order_copy.price, old_order_copy.quantity, old_order_copy.side);
    old_order.active = false;

    if (newId >= global_orders_.size()) [[unlikely]] {
        global_orders_.resize(newId * 2);
    }
    
    global_orders_[newId] = Order{newId, price, qty, old_order_copy.side, old_order_copy.symbol, true};

    book.add_level_quantity(price, qty, old_order_copy.side);
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
