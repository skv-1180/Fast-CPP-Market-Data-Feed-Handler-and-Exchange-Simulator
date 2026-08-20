#include "moldudp64/moldudp64.h"
#include "common/utility.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

MoldUDP64Packet::MoldUDP64Packet(const std::string& session, SeqNo sequence_number)
    : session_{session}, sequence_number_{sequence_number}, message_count_{0}
{
    if (session.size() > SESSION_SIZE) {
        throw std::invalid_argument("MoldUDP64 session must be at most 10 bytes");
    }

    buffer_.resize(HEADER_SIZE, ' ');
    std::memcpy(buffer_.data(), session.data(), session.size());

    md::write_u64(buffer_.data() + 10, sequence_number);
    md::write_u16(buffer_.data() + 18, 0);
}

bool MoldUDP64Packet::add_message(const std::uint8_t* data, std::uint16_t size)
{
    if (message_count_ == END_OF_SESSION || message_count_ == END_OF_SESSION - 1)
        return false;

    if (size > 0 && data == nullptr)
        return false;

    const std::size_t old_size = buffer_.size();

    buffer_.resize(old_size + 2 + size); // 2 - for message length

    md::write_u16(buffer_.data() + old_size, size);

    if (size > 0) {
        std::memcpy(buffer_.data() + old_size + 2, data, size);
    }

    ++message_count_;
    md::write_u16(buffer_.data() + 18, message_count_); // update the header

    return true;
}

MoldUDP64Packet MoldUDP64Packet::heartbeat(
    const std::string& session, SeqNo sequence_number)
{
    return MoldUDP64Packet(session, sequence_number);
}

MoldUDP64Packet MoldUDP64Packet::end_of_session(
    const std::string& session, SeqNo sequence_number)
{
    MoldUDP64Packet packet(session, sequence_number);

    packet.message_count_ = END_OF_SESSION;

    md::write_u16(packet.buffer_.data() + 18, END_OF_SESSION);

    return packet;
}

const std::uint8_t* MoldUDP64Packet::data() const 
{
    return buffer_.data(); 
}

std::size_t MoldUDP64Packet::size() const 
{
    return buffer_.size(); 
}

SeqNo MoldUDP64Packet::sequence_number() const
{
    return sequence_number_;
}

std::uint16_t MoldUDP64Packet::message_count() const 
{
    return message_count_; 
}

const std::string& MoldUDP64Packet::session() const 
{
    return session_; 
}
