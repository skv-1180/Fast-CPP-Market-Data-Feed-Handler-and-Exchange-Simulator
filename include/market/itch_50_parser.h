#pragma once

#include "common/types.h"
#include <cstddef>
#include <cstdint>
#include <bit>
#include <array>
#include <iostream>

template <typename Market> 
class Itch50Parser {
public:
    Itch50Parser(Market& market);

    std::size_t parse_multiple_message(const std::uint8_t* msg_buffer, std::size_t len);

    void parse_single_message(const std::uint8_t* msg_buffer, std::size_t len);

    std::uint64_t message_count() const;

private:
    using MsgHandler = void (Itch50Parser::*)(const std::uint8_t*);

    void add_order(const std::uint8_t* msg_buffer);

    void executed_order(const std::uint8_t* msg_buffer);

    void executed_at_price_order(const std::uint8_t* msg_buffer);

    void cancel_order(const std::uint8_t* msg_buffer);

    void delete_order(const std::uint8_t* msg_buffer);

    void replace_order(const std::uint8_t* msg_buffer);

    void no_op([[maybe_unused]] const std::uint8_t* msg_buffer) {} // dummy function

    Market& market_;
    std::uint64_t messages_ = 0; // for measuring performance
    std::array<MsgHandler, 256> dispatch_table_;
};

template <typename Market>
Itch50Parser<Market>::Itch50Parser(Market& market)
    : market_ { market }
{
    dispatch_table_.fill(&Itch50Parser::no_op);

    dispatch_table_['A'] = &Itch50Parser::add_order;
    dispatch_table_['F'] = &Itch50Parser::add_order;
    dispatch_table_['U'] = &Itch50Parser::replace_order;
    dispatch_table_['D'] = &Itch50Parser::delete_order;
    dispatch_table_['E'] = &Itch50Parser::executed_order;
    dispatch_table_['X'] = &Itch50Parser::cancel_order;
    dispatch_table_['C'] = &Itch50Parser::executed_at_price_order;
}

template <typename Market>
std::size_t Itch50Parser<Market>::parse_multiple_message(
    const std::uint8_t* msg_buffer, std::size_t len)
{
    std::size_t i = 0;
    while (i < len) {
        std::uint16_t msg_len = std::byteswap(*reinterpret_cast<const std::uint16_t*>(msg_buffer+i));
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

    const std::uint8_t type = msg_buffer[0];

    (this->*dispatch_table_[type])(msg_buffer);
}

template <typename Market>
inline void Itch50Parser<Market>::add_order(const std::uint8_t* msg_buffer)
{
    const auto* msg = reinterpret_cast<const ItchAddOrder*>(msg_buffer);

    OrderId order_id = std::byteswap(msg->order_reference_number);
    Side side = (msg->buy_sell_indicator == 'B') ? Side::Buy : Side::Sell;
    Quantity quantity = std::byteswap(msg->shares);
    Price price = std::byteswap(msg->price);
    // Symbol symbol = std::byteswap(msg->stock);
    SymbolId symbol_id = std::byteswap(msg->stock_locate);

    market_.add_order(order_id, side, quantity, symbol_id, price);
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
