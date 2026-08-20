#pragma once
#include <cstdint>

using SeqNo = std::uint64_t;
constexpr std::size_t HEADER_SIZE = 20;
constexpr std::size_t SESSION_SIZE = 10;
constexpr std::uint16_t END_OF_SESSION = 0xFFFF;
