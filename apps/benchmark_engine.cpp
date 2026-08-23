#include "market/itch_50_parser.h"
#include "market/market.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char* argv[]) 
{
    if (argc != 2) return 1;

    std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<std::uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    Market market;
    Itch50Parser<Market> itch_parser(market);

    const std::uint8_t* ptr = buffer.data();
    const std::uint8_t* end = buffer.data() + size;
    std::uint64_t total_messages = 0;

    auto start_time = std::chrono::steady_clock::now();

    while (ptr + 2 <= end) 
    {
        std::uint16_t msg_length = std::byteswap(*reinterpret_cast<const std::uint16_t*>(ptr));
        ptr += 2;

        if (msg_length == 0 || ptr + msg_length > end) [[unlikely]]
            break;

        itch_parser.parse_single_message(ptr, msg_length);
        
        ptr += msg_length;
        ++total_messages;
    }

    auto end_time = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(end_time - start_time).count();

    std::cout << "Processed:  " << total_messages << " messages\n";
    std::cout << "Time:       " << elapsed_sec << " seconds\n";
    std::cout << "Throughput: " << (total_messages / elapsed_sec) / 1e6 << " M msg/sec\n";
    std::cout << "Latency:    " << (elapsed_sec * 1e9) / total_messages << " ns / msg\n";

    return 0;
}