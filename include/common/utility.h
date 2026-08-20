#pragma once

#include <cstdint>
#include <string>
#include "types.h"
#include <cstring>

// Assumption: Host Machine is Little-Endian
namespace md  // market data
{
inline std::uint16_t read_16_Bit(const void* buf)
{
    const auto* src = static_cast<const std::uint8_t*>(buf);
    
    return (static_cast<std::uint16_t>(src[0]) << 8)
        |  static_cast<std::uint16_t>(src[1]);
}

inline std::uint32_t read_32_bit(const void* buf)
{
    const auto* src = static_cast<const std::uint8_t*>(buf);
    std::uint32_t value = 0;

    for (int i = 0; i < 4; ++i) {
        value = (value << 8) | static_cast<std::uint32_t>(src[i]);
    }

    return value;
}

inline std::uint64_t read_64_bit(const void* buf)
{
    const auto* src = static_cast<const std::uint8_t*>(buf);
    std::uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value = (value << 8) | static_cast<std::uint64_t>(src[i]);
    }

    return value;
}

inline std::uint64_t read_sym_64_bit(const void* buf)
{
    return read_64_bit(buf);
}

inline void write_u16(std::uint8_t* dst, std::uint16_t value)
{
    dst[0] = static_cast<std::uint8_t>(value >> 8);
    dst[1] = static_cast<std::uint8_t>(value);
}

inline void write_u64(std::uint8_t* dst, std::uint64_t value)
{
    for (int i = 7; i >= 0; --i) {
        dst[i] = static_cast<std::uint8_t>(value & 0xFF);
        value >>= 8;
    }
}

inline std::string symbol_to_string(Symbol sym)
{
    sym = __builtin_bswap64(sym);

    char buf[9];
    std::memcpy(buf, &sym, 8);
    buf[8] = '\0';

    std::string s(buf, 8);
    while (!s.empty() && s.back() == ' ')
        s.pop_back();

    return s;
}
}  // namespace md

/*
inline uint32_t read_16_Bit(const void* buf)
{
    return __builtin_bswap16(*static_cast<const uint16_t*>(buf));
}

inline uint32_t read_32_bit(const void* buf)
{
    return __builtin_bswap32(*static_cast<const uint32_t*>(buf));
}

inline uint64_t read_64_bit(const void* buf)
{
    return __builtin_bswap64(*static_cast<const uint64_t*>(buf));
}

inline uint64_t read_sym_64_bit(const void* buf)
{
    return __builtin_bswap64(*static_cast<const uint64_t*>(buf));
}
*/
