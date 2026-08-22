#pragma once

#include "common/types.h"
#include "market/order_book.h"
#include <unordered_map>
#include <vector>

struct Order 
{
    OrderId order_id;
    Price price;
    Quantity quantity;
    Side side;
    Symbol symbol; 
    bool active = false;
};

class Market {
public:
    Market(std::size_t max_orders = 5'000'000);

    void add_order(OrderId order_id, Side side, Quantity qty,
                   Symbol symbol, Price price);

    void executed_order(OrderId order_id, Quantity qty);
    void executed_at_price_order(OrderId order_id, Quantity qty, Price price);
    void cancel_order(OrderId order_id, Quantity qty);
    void delete_order(OrderId order_id);
    void replace_order(OrderId oldId, OrderId newId,
                       Quantity qty, Price price);

    const OrderBook* find_book(Symbol symbol) const;
    void print_best_bid_ask() const; // debugging

    Market(const Market&) = delete;
    Market& operator=(const Market&) = delete;
    Market(Market&&) = delete;
    Market& operator=(Market&&) = delete;

private:
    void reduce_order_size(OrderId order_id, Quantity qty); 
    
    std::unordered_map<Symbol, OrderBook> books_;    
    std::vector<Order> global_orders_;
};
