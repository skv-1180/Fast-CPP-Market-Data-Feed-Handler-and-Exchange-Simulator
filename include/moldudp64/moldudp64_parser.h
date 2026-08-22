#pragma once

#include "moldudp64/moldudp64_protocol.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class MoldUDP64Parser 
{
public:
    struct Message {
        const std::uint8_t* data;
        std::uint16_t size;
    };

    MoldUDP64Parser() = default;
    
    bool parse(const std::uint8_t* data, std::size_t size);

    const std::string& session() const;

    SeqNo sequence_number() const;

    std::uint16_t message_count() const;

    Message message(std::size_t index) const;

    bool is_heartbeat() const;

    bool is_end_of_session() const;

    MoldUDP64Parser(const MoldUDP64Parser&) = delete;
    MoldUDP64Parser& operator= (const MoldUDP64Parser&) = delete;

    MoldUDP64Parser(MoldUDP64Parser&&) = delete;
    MoldUDP64Parser& operator= (MoldUDP64Parser&&) = delete;

private:
    const std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;

    std::string session_;
    SeqNo sequence_number_ = 0;
    std::uint16_t message_count_ = 0;

    std::vector<std::size_t> message_offsets_;
};