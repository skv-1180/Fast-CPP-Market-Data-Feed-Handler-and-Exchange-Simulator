#include "exchange/exchange.h"

#include <limits>
#include <stdexcept>

Exchange::Exchange(
    const char* host, 
    const char* port,
    const std::string& session, 
    SeqNo initial_sequence,
    std::size_t max_packet_size)
    : client_ { host, port }
    , session_ { session }
    , next_sequence_ { initial_sequence }
    , max_packet_size_ { max_packet_size }
    , packet_ { session, initial_sequence }
{
    if (!client_.success())
        throw std::runtime_error("Failed to initialize UDP client");

    if (max_packet_size_ < HEADER_SIZE)
        throw std::invalid_argument(
            "max_packet_size must be at least HEADER_SIZE");
}

bool Exchange::publish(const std::uint8_t* data, std::size_t size)
{

    if (size > 0 && data == nullptr)
        return false;

    const std::size_t required_size = 2 + size;

    
    // If the message itself cannot fit inside a packet, it cannot be published.
    if (HEADER_SIZE + required_size > max_packet_size_) {
        return false;
    }

    
    // If adding this message would make the current packet too large, 
    // send the current packet first.
    if (packet_.size() + required_size > max_packet_size_) {
        if (!flush())
            return false;
    }

    if (!packet_.add_message(data, size))
        return false;

    ++messages_sent_;
    ++next_sequence_;

    return true;
}

bool Exchange::flush()
{
    if (packet_.message_count() == 0)
        return true;

    if (client_.send(packet_.data(), packet_.size()) != packet_.size()) 
        return false;

    ++packets_sent_;

    packet_ = MoldUDP64Packet(session_, next_sequence_);

    return true;
}

bool Exchange::send_heartbeat()
{
    if (!flush())
        return false;

    MoldUDP64Packet heartbeat = MoldUDP64Packet::heartbeat(session_, next_sequence_);

    if (client_.send(heartbeat.data(), heartbeat.size()) != heartbeat.size())
        return false;

    ++packets_sent_;

    return true;
}

bool Exchange::send_end_of_session()
{
    if (!flush())
        return false;

    MoldUDP64Packet packet = MoldUDP64Packet::end_of_session(session_, next_sequence_);

    if (client_.send(packet.data(), packet.size()) != packet.size())
        return false;

    ++packets_sent_;

    return true;
}

SeqNo Exchange::next_sequence() const 
{ 
    return next_sequence_; 
}

std::uint64_t Exchange::packets_sent() const 
{ 
    return packets_sent_; 
}

std::uint64_t Exchange::messages_sent() const 
{ 
    return messages_sent_; 
}
