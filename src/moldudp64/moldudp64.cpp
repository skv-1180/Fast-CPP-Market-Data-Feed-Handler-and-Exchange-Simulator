#include "moldudp64/moldudp64.h"
#include "common/utility.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <bit>

MoldUDP64Packet::MoldUDP64Packet(std::string_view session, SeqNo sequence_number)
    : sequence_number_{sequence_number}, message_count_{0}
{
    if (session.size() > mold::SESSION_SIZE) {
        throw std::invalid_argument("MoldUDP64 session must be at most 10 bytes");
    }

    auto* header = reinterpret_cast<MoldUDP64Header*>(buffer_.data());
    
    std::memset(header->session, ' ', mold::SESSION_SIZE);
    std::memcpy(header->session, session.data(), session.size());
    
    header->sequence_number = std::byteswap(sequence_number);
    header->message_count = 0; 

    current_size_ = mold::HEADER_SIZE;
}

bool MoldUDP64Packet::has_capacity_for(std::uint16_t size) const
{
    return (current_size_ + 2 + size) <= mold::MAX_PACKET_SIZE;
}


bool MoldUDP64Packet::add_message(const std::uint8_t* data, std::uint16_t size)
{
    if (message_count_ == mold::END_OF_SESSION || message_count_ == mold::END_OF_SESSION - 1) [[unlikely]]
        return false;

    if (size > 0 && data == nullptr) [[unlikely]]
        return false;

    if (!has_capacity_for(size)) [[unlikely]]
        return false;

    std::uint16_t net_size = std::byteswap(size);
    std::memcpy(buffer_.data() + current_size_, &net_size, sizeof(net_size));
    
    if (size > 0) [[likely]]
    {
        std::memcpy(buffer_.data() + current_size_ + 2, data, size);
    }

    current_size_ += 2 + size;
    ++message_count_;

    auto* header = reinterpret_cast<MoldUDP64Header*>(buffer_.data());
    header->message_count = std::byteswap(message_count_);

    return true;
}

MoldUDP64Packet MoldUDP64Packet::heartbeat(std::string_view session, SeqNo sequence_number)
{
    return MoldUDP64Packet{session, sequence_number};
}

MoldUDP64Packet MoldUDP64Packet::end_of_session(std::string_view session, SeqNo sequence_number)
{
    MoldUDP64Packet packet{session, sequence_number};

    packet.message_count_ = mold::END_OF_SESSION;

    auto* header = reinterpret_cast<MoldUDP64Header*>(packet.buffer_.data());
    header->message_count = std::byteswap(mold::END_OF_SESSION);

    return packet;
}

const std::uint8_t* MoldUDP64Packet::data() const 
{
    return buffer_.data(); 
}

std::size_t MoldUDP64Packet::size() const 
{
    return current_size_; 
}

SeqNo MoldUDP64Packet::sequence_number() const
{
    return sequence_number_;
}

std::uint16_t MoldUDP64Packet::message_count() const 
{
    return message_count_; 
}

std::string MoldUDP64Packet::session() const 
{
    const auto* header = reinterpret_cast<const MoldUDP64Header*>(buffer_.data());
    
    int len = mold::SESSION_SIZE;
    while (len > 0 && header->session[len - 1] == ' ') {
        --len;
    }
    
    return std::string(header->session, len);
}

void MoldUDP64Packet::clear(SeqNo next_sequence) 
{
    auto* header = reinterpret_cast<MoldUDP64Header*>(buffer_.data());
    header->sequence_number = std::byteswap(next_sequence);
    header->message_count = 0;
    
    sequence_number_ = next_sequence;
    message_count_ = 0;
    current_size_ = mold::HEADER_SIZE;
}