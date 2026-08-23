#include "market/itch_50_parser.h"
#include "market/market.h"
#include "moldudp64/moldudp64_parser.h"
#include "network/udp_server.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <vector>

namespace 
{
    __attribute__((noinline, cold))
    void log_invalid_packet() {
        std::cerr << "Invalid MoldUDP64 packet\n";
    }

    __attribute__((noinline, cold))
    void log_heartbeat(std::uint64_t sequence) {
        std::cout << "Heartbeat: sequence=" << sequence << '\n';
    }

    __attribute__((noinline, cold))
    void log_sequence_gap(std::uint64_t expected, std::uint64_t received) {
        std::cerr << "Sequence gap/error: expected " << expected
                << ", received " << received << '\n';
    }
}

void run_market_data_consumer(const char* port )
{
    constexpr std::size_t BUFFER_SIZE = 65536;
    
    UdpServer server(port);
    std::cout << "Feed handler started on port " << port << '\n';

    Market market;
    Itch50Parser<Market> itch_parser(market);
    MoldUDP64Parser mold_parser;

    std::uint64_t expected_sequence = mold::INITIAL_SEQUENCE;
    std::uint64_t packets_received = 0;
    std::uint64_t messages_received = 0;
    std::uint64_t total_parse_time_ns = 0;

    std::vector<std::uint8_t> buffer(BUFFER_SIZE);

    while (true) {
        std::size_t received = server.receive(buffer.data(), buffer.size());

        if (received == 0) [[unlikely]]
        {
            continue;
        }

        auto start = std::chrono::steady_clock::now();
        
        ++packets_received;

        if (!mold_parser.parse(buffer.data(), received)) [[unlikely]]
        {
            log_invalid_packet();
            continue;
        }

        const std::uint64_t sequence = mold_parser.sequence_number();
        const std::uint16_t message_count = mold_parser.message_count();

        if (mold_parser.is_heartbeat()) [[unlikely]]
        {
            log_heartbeat(sequence);
            continue;
        }

        if (mold_parser.is_end_of_session()) [[unlikely]]
        {
            break;
        }

        if (sequence != expected_sequence) [[unlikely]]
        {
            log_sequence_gap(expected_sequence, sequence);
        }

        for (std::size_t i = 0; i < message_count; ++i) {
            auto message = mold_parser.message(i);
            itch_parser.parse_single_message(message.data, message.size);
        }

        messages_received += message_count;
        expected_sequence = sequence + message_count;

        auto end = std::chrono::steady_clock::now();
        total_parse_time_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }

    std::cout << "Feed handler stopped\n";
    std::cout << "Packets received:  " << packets_received << '\n';
    std::cout << "Messages received: " << messages_received << '\n';
    std::cout << "Messages parsed:   " << itch_parser.message_count() << '\n';
    std::cout << "Next sequence:     " << expected_sequence << '\n';

    std::cout << "ITCH parse time:   " << total_parse_time_ns / 1e9 << " s\n";

    std::cout << "ITCH throughput:   " 
        << (messages_received / (total_parse_time_ns / 1e9)) / 1e6
        << " M msg/s\n";
    

    market.print_best_bid_ask();
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: market_data_consumer <port>\n";
        return 1;
    }

    const char* port = argv[1];

    try {
        run_market_data_consumer(port);
    } 
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    } 

    return 0;
}