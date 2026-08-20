#include "moldudp64/moldudp64_parser.h"
#include "common/utility.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

bool MoldUDP64Parser::parse(const std::uint8_t* data, std::size_t size)
{
    data_ = data;
    size_ = size;

    session_.clear();
    sequence_number_ = 0;
    message_count_ = 0;
    message_offsets_.clear();

    if (data_ == nullptr || size_ < HEADER_SIZE) 
    {
        return false;
    }

    session_.assign(reinterpret_cast<const char*>(data_), SESSION_SIZE);
    while (!session_.empty() && session_.back() == ' ') 
    {
        session_.pop_back();
    }

    sequence_number_ = md::read_64_bit(data_ + 10);
    message_count_ = md::read_16_bit(data_ + 18);

    // heartbeat or end_of_session packet
    if (message_count_ == 0 || message_count_ == END_OF_SESSION) 
    {
        return size_ == HEADER_SIZE;
    }

    std::size_t offset = HEADER_SIZE;

    message_offsets_.reserve(message_count_);

    for (std::uint16_t i = 0; i < message_count_; ++i) 
    {
        if (offset + 2 > size_) 
        {
            return false;
        }

        const std::uint16_t message_size = md::read_16_bit(data_ + offset);
        offset += 2;

        if (offset + message_size > size_) 
        {
            return false;
        }

        message_offsets_.push_back(offset);
        offset += message_size;
    }

    if (offset != size_) {
        return false;
    }

    return true;
}

const std::string& MoldUDP64Parser::session() const 
{ 
    return session_; 
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
    if (index >= message_offsets_.size()) {
        throw std::out_of_range("MoldUDP64 message index out of range");
    }

    const std::size_t offset = message_offsets_[index];
    const std::uint16_t message_size = md::read_16_bit(data_ + offset - 2);

    return Message { data_ + offset, message_size };
}

bool MoldUDP64Parser::is_heartbeat() const 
{ 
    return message_count_ == 0; 
}

bool MoldUDP64Parser::is_end_of_session() const
{
    return message_count_ == END_OF_SESSION;
}