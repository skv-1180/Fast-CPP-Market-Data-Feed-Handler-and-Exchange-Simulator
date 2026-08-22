#pragma once

#include "moldudp64/moldudp64_protocol.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class MoldUDP64Packet 
{
   public:
    MoldUDP64Packet(const std::string& session, SeqNo sequence_number);

    bool add_message(const std::uint8_t* data, std::uint16_t size);                                 

    static MoldUDP64Packet heartbeat(const std::string& session, SeqNo sequence_number);

    static MoldUDP64Packet end_of_session(const std::string& session, SeqNo sequence_number);

    const std::uint8_t* data() const;
    std::size_t size() const;

    SeqNo sequence_number() const;
    std::uint16_t message_count() const;

    const std::string& session() const;

    MoldUDP64Packet(const MoldUDP64Packet&) = delete;
    MoldUDP64Packet& operator=(const MoldUDP64Packet&) = delete;

    MoldUDP64Packet (MoldUDP64Packet&&) = default;
    MoldUDP64Packet& operator=(MoldUDP64Packet&&) = default;

   private:
    std::vector<std::uint8_t> buffer_;

    std::string session_;
    SeqNo sequence_number_;
    std::uint16_t message_count_;
};
