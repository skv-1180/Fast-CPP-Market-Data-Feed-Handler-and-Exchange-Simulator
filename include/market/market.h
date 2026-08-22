#pragma once

#include "common/types.h"
#include "common/config.h"
#include "market/order_book.h"
#include <unordered_map>
#include <vector>
#include <array>

struct alignas(config::CACHE_LINE_SIZE) Order 
{
    OrderId order_id;
    Price price;
    Quantity quantity;
    Side side;
    SymbolId symbol_id; 
    bool active = false;
};

class Market {
public:
    Market(std::size_t max_orders = config::INITIAL_ORDER_CAPACITY);

    void add_order(OrderId order_id, Side side, Quantity qty, SymbolId symbol_id, Price price);
    void executed_order(OrderId order_id, Quantity qty);
    void executed_at_price_order(OrderId order_id, Quantity qty, Price price);
    void cancel_order(OrderId order_id, Quantity qty);
    void delete_order(OrderId order_id);
    void replace_order(OrderId oldId, OrderId newId, Quantity qty, Price price);

    const OrderBook* find_book(SymbolId symbol_id) const;
    void print_best_bid_ask() const; // debugging

    Market(const Market&) = delete;
    Market& operator=(const Market&) = delete;
    Market(Market&&) = delete;
    Market& operator=(Market&&) = delete;

private:
    void reduce_order_size(OrderId order_id, Quantity qty); 
    
    std::array<OrderBook, config::MAX_MARKET_SYMBOLS> books_;
    std::vector<Order> global_orders_;
};
