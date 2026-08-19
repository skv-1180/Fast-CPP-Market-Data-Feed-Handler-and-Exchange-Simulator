#pragma once

#include <cstdint>
#include <string>
#include "types.h"
#include <cstring>

namespace md  // market data
{
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