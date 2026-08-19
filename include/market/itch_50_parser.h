#pragma once

#include <cstddef>
#include <cstdint>

#include "market/utility.h"
#include "market/types.h"

template <typename Market>
class Itch50Parser
{
   public:
    Itch50Parser(Market& market)
        : market_{market}
    {
    }

    size_t parse_multiple_message(const char* msg_buffer, size_t len)
    {
        using namespace md;
        size_t i = 0;

        while (i < len)
        {
            size_t msg_len = read_16_Bit(msg_buffer + i);
            if (i + 2 + msg_len > len)
            {
                break;
            }
            parse_single_message(msg_buffer + 2 + i);
            i += msg_len + 2;
        }

        return i;
    }

    void parse_single_message(const char* msg_buffer)
    {
        ++messages_;
        char type = msg_buffer[0];
        switch (type)
        {
            case 'A':
                return add_order(msg_buffer);
            case 'F':
                return add_order(msg_buffer);
            case 'E':
                return executed_order(msg_buffer);
            case 'C':
                return executed_at_price_order(msg_buffer);
            case 'X':
                return cancel_order(msg_buffer);
            case 'D':
                return delete_order(msg_buffer);
            case 'U':
                return replace_order(msg_buffer);
        }
    }

    uint64_t message_count() const
    {
        return messages_;
    }

   private:
    void add_order(const char* msg_buffer)
    {
        using namespace md;
        OrderId order_id = read_64_bit(msg_buffer + 11);
        Side side = (msg_buffer[19] == 'B') ? Side::Buy : Side::Sell;
        Quantity quantity = read_32_bit(msg_buffer + 20);
        Symbol symbol = read_sym_64_bit(msg_buffer + 24);
        Price price = read_32_bit(msg_buffer + 32);

        market_.add_order(order_id, side, quantity, symbol, price);
    }

    void executed_order(const char* msg_buffer)
    {
        using namespace md;
        OrderId order_id = read_64_bit(msg_buffer + 11);
        Quantity quantity = read_32_bit(msg_buffer + 19);
        market_.executed_order(order_id, quantity);
    }

    void executed_at_price_order(const char* msg_buffer)
    {
        using namespace md;
        OrderId order_id = read_64_bit(msg_buffer + 11);
        Quantity quantity = read_32_bit(msg_buffer + 19);
        Price price = read_32_bit(msg_buffer + 32);
        market_.executed_at_price_order(order_id, quantity, price);
    }

    void cancel_order(const char* msg_buffer)
    {
        using namespace md;
        OrderId order_id = read_64_bit(msg_buffer + 11);
        Quantity quantity = read_32_bit(msg_buffer + 19);
        market_.cancel_order(order_id, quantity);
    }

    void delete_order(const char* msg_buffer)
    {
        using namespace md;
        OrderId order_id = read_64_bit(msg_buffer + 11);
        market_.delete_order(order_id);
    }

    void replace_order(const char* msg_buffer)
    {
        using namespace md;
        OrderId old_id = read_64_bit(msg_buffer + 11);
        OrderId new_id = read_64_bit(msg_buffer + 19);
        Quantity quantity = read_32_bit(msg_buffer + 27);
        Price price = read_32_bit(msg_buffer + 31);
        market_.replace_order(old_id, new_id, quantity, price);
    }

    Market& market_;
    uint64_t messages_ = 0; // for measuring performance
};