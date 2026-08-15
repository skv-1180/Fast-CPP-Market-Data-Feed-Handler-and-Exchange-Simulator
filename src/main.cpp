#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "itch_50_parser.h"
#include "market.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: replay <itch_file>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file)
    {
        std::cerr << "Cannot open file: " << argv[1] << '\n';
        return 1;
    }

    Market market;
    Itch50Parser<Market> parser(market);

    constexpr size_t BUFFER_SIZE = 1 << 20;   // 1 MB
    std::vector<char> buffer(BUFFER_SIZE * 2);

    size_t leftover = 0;

    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        file.read(buffer.data() + leftover, BUFFER_SIZE);

        std::streamsize n = file.gcount();
        if (n <= 0)
            break;

        size_t bytes = leftover + static_cast<size_t>(n);

        size_t consumed = parser.parse_multiple_message(buffer.data(), bytes);

        if (consumed > bytes)
        {
            std::cerr << "Parser error: consumed "
                      << consumed
                      << " bytes from buffer of "
                      << bytes
                      << " bytes\n";
            return 1;
        }

        leftover = bytes - consumed;

        if (leftover > 0)
        {
            std::memmove(buffer.data(),
                         buffer.data() + consumed,
                         leftover);
        }
    }

    auto end = std::chrono::steady_clock::now();

    double sec = std::chrono::duration<double>(end - start).count();
    uint64_t msgs = parser.message_count();

    std::cout << "Messages:   " << msgs << '\n';
    std::cout << "Time:       " << sec << " s\n";
    std::cout << "Throughput: "
              << (msgs / sec) / 1e6
              << " M msg/s\n";

    if (leftover != 0)
    {
        std::cout << "Leftover bytes: " << leftover << '\n';
    }

    market.print_best_bid_ask();

    return 0;
}