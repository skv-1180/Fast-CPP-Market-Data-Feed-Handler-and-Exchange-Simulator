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

    constexpr size_t BUFFER_SIZE = 1 << 20;   // 1 MB
    constexpr int REPEATS = 10;

    std::vector<char> buffer(BUFFER_SIZE * 2);

    std::chrono::duration<double> parse_time{0};
    uint64_t total_msgs = 0;

    for (int iter = 0; iter < REPEATS; ++iter)
    {
        file.clear();
        file.seekg(0);

        Market market;
        Itch50Parser<Market> parser(market);

        size_t leftover = 0;

        while (true)
        {
            file.read(buffer.data() + leftover, BUFFER_SIZE);

            std::streamsize n = file.gcount();
            if (n <= 0)
                break;

            size_t bytes = leftover + static_cast<size_t>(n);

            auto start = std::chrono::steady_clock::now();
            size_t consumed = parser.parse_multiple_message(buffer.data(), bytes);
            auto end = std::chrono::steady_clock::now();
            parse_time += (end - start);

            if (consumed > bytes)
            {
                std::cerr << "Parser error: consumed " << consumed
                          << " bytes from buffer of " << bytes << " bytes\n";
                return 1;
            }

            leftover = bytes - consumed;

            if (leftover > 0)
            {
                std::memmove(buffer.data(), buffer.data() + consumed, leftover);
            }
        }

        total_msgs += parser.message_count();

        // if (!iter)
        //     market.print_best_bid_ask();
    }

    double sec = parse_time.count();

    std::cout << "Messages:   " << total_msgs << '\n';
    std::cout << "Time:       " << sec << " s\n";
    std::cout << "Throughput: " << (total_msgs / sec) / 1e6 << " M msg/s\n";

    return 0;
}