#include <cstdint>

#include "itch_50_parser.h"
#include "endian.h"
#include "types.h"

template <typename OrderBook>
size_t Itch50Parser<OrderBook>::parse_multiple_message(
    const char* msg_buffer, size_t len)
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
        parse_single_message(msg_buffer + 2);
        i += msg_len + 2;
    }

    return i; 
}

template <typename OrderBook>
void Itch50Parser<OrderBook>::parse_single_message(const char* msg_buffer)
{
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

template <typename OrderBook>
void Itch50Parser<OrderBook>::add_order(const char* msg_buffer)
{
    using namespace md;
    OrderId order_id = read_64_bit(msg_buffer + 11);
    Side side = (msg_buffer[19] == 'B') ? Side::Buy : Side::Sell;
    Quantity quantity = read_32_bit(msg_buffer + 20);
    Symbol symbol = read_sym_64_bit(msg_buffer + 24);
    Price price = read_32_bit(msg_buffer + 32);

    order_book_.add_order(order_id, side, quantity, symbol, price);
}

template <typename OrderBook>
void Itch50Parser<OrderBook>::executed_order(const char* msg_buffer)
{
    using namespace md;
    OrderId order_id = read_64_bit(msg_buffer + 11);
    Quantity quantity = read_32_bit(msg_buffer + 19);
    order_book_.executed_order(order_id, quantity);
}

template <typename OrderBook>
void Itch50Parser<OrderBook>::executed_at_price_order(const char* msg_buffer)
{
    using namespace md;
    OrderId order_id = read_64_bit(msg_buffer + 11);
    Quantity quantity = read_32_bit(msg_buffer + 19);
    Price price = read_32_bit(msg_buffer + 32);
    order_book_.executed_at_price_order(order_id, quantity, price);
}

template <typename OrderBook>
void Itch50Parser<OrderBook>::cancel_order(const char* msg_buffer)
{
    using namespace md;
    OrderId order_id = read_64_bit(msg_buffer + 11);
    Quantity quantity = read_32_bit(msg_buffer + 19);
    order_book_.cancel_order(order_id, quantity);
}

template <typename OrderBook>
void Itch50Parser<OrderBook>::delete_order(const char* msg_buffer)
{
    using namespace md;
    OrderId order_id = read_64_bit(msg_buffer + 11);
    order_book_.delete_order(order_id);
}

template <typename OrderBook>
void Itch50Parser<OrderBook>::replace_order(const char* msg_buffer)
{
    using namespace md;
    OrderId original_order_id = read_64_bit(msg_buffer + 11);
    OrderId new_order_id = read_64_bit(msg_buffer + 19);
    Quantity quantity = read_32_bit(msg_buffer + 27);
    Price price = read_32_bit(msg_buffer + 31);
    order_book_.replace_order(original_order_id, new_order_id, quantity, price);
}

int main(){}