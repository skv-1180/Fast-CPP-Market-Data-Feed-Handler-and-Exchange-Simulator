#pragma once

#include "moldudp64/moldudp64_protocol.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

class MoldUDP64Packet {
public:
    MoldUDP64Packet(std::string_view session, SeqNo sequence_number);

    bool add_message(const std::uint8_t* data, std::uint16_t size);

    bool has_capacity_for(std::uint16_t size) const;

    static MoldUDP64Packet heartbeat(std::string_view session, SeqNo sequence_number);

    static MoldUDP64Packet end_of_session(std::string_view session, SeqNo sequence_number);

    const std::uint8_t* data() const;
    std::size_t size() const;

    SeqNo sequence_number() const;
    std::uint16_t message_count() const;

    std::string session() const;

    void clear(SeqNo next_sequence);

    MoldUDP64Packet(const MoldUDP64Packet&) = delete;
    MoldUDP64Packet& operator=(const MoldUDP64Packet&) = delete;

    MoldUDP64Packet(MoldUDP64Packet&&) = default;
    MoldUDP64Packet& operator=(MoldUDP64Packet&&) = default;

private:
    std::array<std::uint8_t, MAX_PACKET_SIZE> buffer_{};
    std::size_t current_size_{0};

    SeqNo sequence_number_;
    std::uint16_t message_count_;
};
