#include "market/order_book.h"

OrderBook::OrderBook()
{
    bids_.reserve(config::EXPECTED_BOOK_DEPTH);
    asks_.reserve(config::EXPECTED_BOOK_DEPTH);
}

void OrderBook::add_level_quantity(Price price, Quantity qty, Side side)
{
    if (side == Side::Buy)
    {
        bids_[price] += qty;
    }
    else
    {
        asks_[price] += qty;
    }
}

void OrderBook::reduce_level_quantity(Price price, Quantity qty, Side side)
{
    if (side == Side::Buy)
    {
        auto it = bids_.find(price);
        if (it != bids_.end()) [[likely]]
        {
            it->second -= qty;
            if (it->second == 0) [[unlikely]]
            {
                bids_.erase(it);
            }
        }
    }
    else
    {
        auto it = asks_.find(price);
        if (it != asks_.end()) [[likely]]
        {
            it->second -= qty;
            if (it->second == 0) [[unlikely]]
            {
                asks_.erase(it);
            }
        }
    }
}

void OrderBook::remove_level(Price price, Side side)
{
    if (side == Side::Buy)
    {
        bids_.erase(price);
    }
    else
    {
        asks_.erase(price);
    }
}

std::optional<Price> OrderBook::best_bid() const
{
    if (bids_.empty()) [[unlikely]]
        return std::nullopt;

    return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const
{
    if (asks_.empty()) [[unlikely]]
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

std::size_t OrderBook::bid_levels() const
{
    return bids_.size();
}

std::size_t OrderBook::ask_levels() const
{
    return asks_.size();
}

bool OrderBook::empty() const
{
    return bids_.empty() && asks_.empty();
}
