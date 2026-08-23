#include "moldudp64/moldudp64_parser.h"
#include "common/utility.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <bit>

bool MoldUDP64Parser::parse(const std::uint8_t* data, std::size_t size)
{
    if (size < sizeof(MoldUDP64Header)) [[unlikely]] 
    {
        return false;
    }

    const auto* header = reinterpret_cast<const MoldUDP64Header*>(data);
    std::memcpy(session_data_.data(), header->session, SESSION_SIZE);

    sequence_number_ = std::byteswap(header->sequence_number);
    message_count_ = std::byteswap(header->message_count);

    if (message_count_ == 0 || message_count_ == END_OF_SESSION) [[unlikely]] 
    {
        return size == sizeof(MoldUDP64Header);
    }

    if (message_count_ > MAX_MESSAGES_PER_PACKET) [[unlikely]] 
    {
        return false; 
    }

    std::size_t offset = sizeof(MoldUDP64Header);

    for (std::uint16_t i = 0; i < message_count_; ++i) 
    {
        if (offset + 2 > size) [[unlikely]] 
        {
            return false;
        }

        std::uint16_t msg_size = std::byteswap(*reinterpret_cast<const std::uint16_t*>(data + offset));
        offset += 2;

        if (msg_size == 0 || offset + msg_size > size) [[unlikely]] 
        {
            return false;
        }

        message_offsets_[i] = static_cast<std::uint16_t>(offset);
        message_sizes_[i] = msg_size;

        offset += msg_size;
    }

    if (offset != size) [[unlikely]] 
    {
        return false;
    }

    data_ = data;
    size_ = size;

    return true;
}

std::string MoldUDP64Parser::session() const
{
    int len = 10;
    while (len > 0 && session_data_[len - 1] == ' ') {
        --len;
    }
    return std::string(session_data_.data(), len);
}

SeqNo MoldUDP64Parser::sequence_number() const
{
    return sequence_number_;
}

std::uint16_t MoldUDP64Parser::message_count() const 
{ 
    return message_count_; 
}

MoldUDP64Parser::Message MoldUDP64Parser::message(std::size_t index) const
{
    // if (index >= message_offsets_.size()) {
    //     throw std::out_of_range("MoldUDP64 message index out of range");
    // }

    const std::size_t offset = message_offsets_[index];

    return Message { data_ + offset, message_sizes_[index] };
}

bool MoldUDP64Parser::is_heartbeat() const 
{ 
    return message_count_ == 0; 
}

bool MoldUDP64Parser::is_end_of_session() const
{
    return message_count_ == END_OF_SESSION;
}