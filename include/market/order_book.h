#pragma once

#include "common/types.h"
#include "common/config.h"
#include <optional>
#include <boost/container/flat_map.hpp>

class OrderBook
{
public:
    OrderBook();

    void add_level_quantity(Price price, Quantity qty, Side side);
    void reduce_level_quantity(Price price, Quantity qty, Side side);
    void remove_level(Price price, Side side);
    
    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;
    TotalQuantity bid_quantity(Price price) const;
    TotalQuantity ask_quantity(Price price) const;
    std::size_t bid_levels() const;
    std::size_t ask_levels() const;

    bool empty() const;

    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(OrderBook&&) = delete;
    OrderBook& operator=(OrderBook&&) = delete;
private:
    
    boost::container::flat_map<Price, TotalQuantity, std::greater<Price>> bids_;   
    boost::container::flat_map<Price, TotalQuantity> asks_;
};
