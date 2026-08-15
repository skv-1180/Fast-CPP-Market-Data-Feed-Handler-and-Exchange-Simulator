#pragma once

#include <cstdint>
#include <cstddef>

template <typename Market>
class Itch50Parser
{
public:
    size_t parse_multiple_message(const char* msg_buffer, size_t len);
    void parse_single_message(const char* msg_buffer);

private:
    void add_order(const char* msg_buffer);
    void executed_order(const char* msg_buffer);
    void executed_at_price_order(const char* msg_buffer);
    void cancel_order(const char* msg_buffer);
    void delete_order(const char* msg_buffer);
    void replace_order(const char* msg_buffer);
    
    Market& market_;
};