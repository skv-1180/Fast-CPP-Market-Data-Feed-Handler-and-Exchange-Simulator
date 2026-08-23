#pragma once

#include "moldudp64/moldudp64_protocol.h"
#include "moldudp64/moldudp64.h"
#include "network/udp_client.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

class Exchange
{
public:
    Exchange(
        const char* host,
        const char* port,
        std::string_view session,
        SeqNo initial_sequence,
        std::size_t max_packet_size);

    bool publish(const std::uint8_t* data, std::uint16_t size);

    bool flush();

    bool send_heartbeat();

    bool send_end_of_session();

    SeqNo next_sequence() const;

    std::uint64_t packets_sent() const;

    std::uint64_t messages_sent() const;

private:
    UdpClient client_;
    MoldUDP64Packet packet_;

    SeqNo next_sequence_;
    std::size_t max_packet_size_;

    std::uint64_t packets_sent_{0};
    std::uint64_t messages_sent_{0};
};