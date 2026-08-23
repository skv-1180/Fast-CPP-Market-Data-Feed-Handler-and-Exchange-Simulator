#pragma once
#include <cstdint>

using SeqNo = std::uint64_t;
inline constexpr std::size_t HEADER_SIZE = 20;
inline constexpr std::size_t SESSION_SIZE = 10;
inline constexpr std::uint16_t END_OF_SESSION = 0xFFFF;
inline constexpr std::size_t MAX_MESSAGES_PER_PACKET = 256;
inline constexpr std::size_t MAX_PACKET_SIZE = 1400;
inline constexpr SeqNo INITIAL_SEQUENCE = 1;;

#pragma pack(push, 1)
struct MoldUDP64Header {
    char session[10];
    SeqNo sequence_number;
    std::uint16_t message_count;
};
#pragma pack(pop)