#pragma once

#include "common/types.h"
#include "common/utility.h"
#include <cstddef>
#include <cstdint>
#include <bit>

template <typename Market> 
class Itch50Parser {
public:
    Itch50Parser(Market& market);

    std::size_t parse_multiple_message(const std::uint8_t* msg_buffer, std::size_t len);

    void parse_single_message(const std::uint8_t* msg_buffer, std::size_t len);

    std::uint64_t message_count() const;

private:
    void add_order(const std::uint8_t* msg_buffer);

    void executed_order(const std::uint8_t* msg_buffer);

    void executed_at_price_order(const std::uint8_t* msg_buffer);

    void cancel_order(const std::uint8_t* msg_buffer);

    void delete_order(const std::uint8_t* msg_buffer);

    void replace_order(const std::uint8_t* msg_buffer);

    Market& market_;
    std::uint64_t messages_ = 0; // for measuring performance
};

template <typename Market>
Itch50Parser<Market>::Itch50Parser(Market& market)
    : market_ { market }
{
}

template <typename Market>
std::size_t Itch50Parser<Market>::parse_multiple_message(
    const std::uint8_t* msg_buffer, std::size_t len)
{
    using namespace md;
    std::size_t i = 0;

    while (i < len) {
        std::size_t msg_len = read_16_bit(msg_buffer + i);

        if (i + 2 + msg_len > len) {
            break;
        }

        parse_single_message(msg_buffer + 2 + i, msg_len);

        i += msg_len + 2;
    }

    return i;
}

template <typename Market>
inline void Itch50Parser<Market>::parse_single_message(
    const std::uint8_t* msg_buffer, [[maybe_unused]] std::size_t len)
{
    ++messages_;

    const char type = static_cast<char>(msg_buffer[0]);
    /*
        'A' - 0
        'F' - 0
        'U' - 1
        |add_order|  
    */
    switch (type) 
    {
        case 'A': [[likely]]
        case 'F': [[likely]]
            add_order(msg_buffer);
            break;

        case 'U': [[likely]]
            replace_order(msg_buffer);
            break;

        case 'D': [[likely]]
            delete_order(msg_buffer);
            break;

        case 'E': [[likely]]
            executed_order(msg_buffer);
            break;

        case 'X':
            cancel_order(msg_buffer);
            break;

        case 'C':
            executed_at_price_order(msg_buffer);
            break;

        default:
            break;
    }
}

template <typename Market>
inline void Itch50Parser<Market>::add_order(const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchAddOrder*>(msg_buffer);

    OrderId order_id = std::byteswap(msg->order_reference_number);
    Side side = (msg->buy_sell_indicator == 'B') ? Side::Buy : Side::Sell;
    Quantity quantity = std::byteswap(msg->shares);
    Price price = std::byteswap(msg->price);
    Symbol symbol = std::byteswap(msg->stock);

    market_.add_order(order_id, side, quantity, symbol, price);
}

template <typename Market>
inline void Itch50Parser<Market>::executed_order(const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchOrderExecuted*>(msg_buffer);

    OrderId order_id = std::byteswap(msg->order_reference_number);
    Quantity quantity = std::byteswap(msg->executed_shares);

    market_.executed_order(order_id, quantity);
}

template <typename Market>
inline void Itch50Parser<Market>::executed_at_price_order(
    const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchOrderExecutedWithPrice*>(msg_buffer);

    OrderId order_id = std::byteswap(msg->order_reference_number);
    Quantity quantity = std::byteswap(msg->executed_shares);
    Price price = std::byteswap(msg->execution_price);

    market_.executed_at_price_order(order_id, quantity, price);
}

template <typename Market>
inline void Itch50Parser<Market>::cancel_order(const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchOrderCancel*>(msg_buffer);

    OrderId order_id = std::byteswap(msg->order_reference_number);
    Quantity quantity = std::byteswap(msg->canceled_shares);

    market_.cancel_order(order_id, quantity);
}

template <typename Market>
inline void Itch50Parser<Market>::delete_order(const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchOrderDelete*>(msg_buffer);

    OrderId order_id = std::byteswap(msg->order_reference_number);

    market_.delete_order(order_id);
}

template <typename Market>
inline void Itch50Parser<Market>::replace_order(const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchOrderReplace*>(msg_buffer);

    OrderId old_id = std::byteswap(msg->original_order_reference_number);
    OrderId new_id = std::byteswap(msg->new_order_reference_number);
    Quantity quantity = std::byteswap(msg->shares);
    Price price = std::byteswap(msg->price);

    market_.replace_order(old_id, new_id, quantity, price);
}

template <typename Market>
std::uint64_t Itch50Parser<Market>::message_count() const
{
    return messages_;
}


// template <typename Market>
// inline void Itch50Parser<Market>::add_order(const std::uint8_t* msg_buffer)
// {
//     using namespace md;

//     OrderId order_id = read_64_bit(msg_buffer + 11);

//     Side side = (static_cast<char>(msg_buffer[19]) == 'B') ? Side::Buy : Side::Sell;

//     Quantity quantity = read_32_bit(msg_buffer + 20);

//     Symbol symbol = read_sym_64_bit(msg_buffer + 24);

//     Price price = read_32_bit(msg_buffer + 32);

//     market_.add_order(order_id, side, quantity, symbol, price);
// }

// template <typename Market>
// inline void Itch50Parser<Market>::executed_order(const std::uint8_t* msg_buffer)
// {
//     using namespace md;

//     OrderId order_id = read_64_bit(msg_buffer + 11);

//     Quantity quantity = read_32_bit(msg_buffer + 19);

//     market_.executed_order(order_id, quantity);
// }

// template <typename Market>
// inline void Itch50Parser<Market>::executed_at_price_order(
//     const std::uint8_t* msg_buffer)
// {
//     using namespace md;

//     OrderId order_id = read_64_bit(msg_buffer + 11);

//     Quantity quantity = read_32_bit(msg_buffer + 19);

//     Price price = read_32_bit(msg_buffer + 32);

//     market_.executed_at_price_order(order_id, quantity, price);
// }

// template <typename Market>
// inline void Itch50Parser<Market>::cancel_order(const std::uint8_t* msg_buffer)
// {
//     using namespace md;

//     OrderId order_id = read_64_bit(msg_buffer + 11);

//     Quantity quantity = read_32_bit(msg_buffer + 19);

//     market_.cancel_order(order_id, quantity);
// }

// template <typename Market>
// inline void Itch50Parser<Market>::delete_order(const std::uint8_t* msg_buffer)
// {
//     using namespace md;

//     OrderId order_id = read_64_bit(msg_buffer + 11);

//     market_.delete_order(order_id);
// }

// template <typename Market>
// inline void Itch50Parser<Market>::replace_order(const std::uint8_t* msg_buffer)
// {
//     using namespace md;

//     OrderId old_id = read_64_bit(msg_buffer + 11);

//     OrderId new_id = read_64_bit(msg_buffer + 19);

//     Quantity quantity = read_32_bit(msg_buffer + 27);

//     Price price = read_32_bit(msg_buffer + 31);

//     market_.replace_order(old_id, new_id, quantity, price);
// }
