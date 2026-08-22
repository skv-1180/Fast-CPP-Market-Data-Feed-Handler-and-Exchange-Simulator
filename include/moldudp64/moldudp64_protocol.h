#pragma once
#include <cstdint>

using SeqNo = std::uint64_t;

inline constexpr std::size_t HEADER_SIZE = 20;
inline constexpr std::size_t SESSION_SIZE = 10;
inline constexpr std::uint16_t END_OF_SESSION = 0xFFFF;
