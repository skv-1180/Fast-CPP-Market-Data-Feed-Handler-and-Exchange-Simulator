#pragma once
#include <cstddef>

namespace config {
    inline constexpr std::size_t CACHE_LINE_SIZE = 64;
    inline constexpr std::size_t INITIAL_ORDER_CAPACITY = 5'000'000;
    inline constexpr std::size_t MAX_MARKET_SYMBOLS = 65'536;
    inline constexpr std::size_t EXPECTED_BOOK_DEPTH = 32;
}