#pragma once

#include <cstdint>

template <typename OrderBook>
class Itch50Parser
{
public:
    using OrderId = uint64_t;
    using Quantity = uint32_t;
    using Price = uint32_t;
    using Symbol = uint64_t;
    enum Side {
        Buy, Sell
    };
    
    size_t parse_multiple_message(const char* msg_buffer, size_t len);
    void parse_single_message(const char* msg_buffer);

private:
    void add_order(const char* msg_buffer);
    void executed_order(const char* msg_buffer);
    void executed_at_price_order(const char* msg_buffer);
    void cancel_order(const char* msg_buffer);
    void delete_order(const char* msg_buffer);
    void replace_order(const char* msg_buffer);
    
    OrderBook& order_book_;
};