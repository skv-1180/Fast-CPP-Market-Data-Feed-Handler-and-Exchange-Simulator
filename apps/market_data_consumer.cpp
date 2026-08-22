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

void run_market_data_consumer(const char* port )
{
    constexpr std::size_t BUFFER_SIZE = 65536;
    
    UdpServer server(port);
    std::cout << "Feed handler started on port " << port << '\n';

    Market market;
    Itch50Parser<Market> itch_parser(market);
    MoldUDP64Parser mold_parser;

    std::uint64_t expected_sequence = 1;
    std::uint64_t packets_received = 0;
    std::uint64_t messages_received = 0;
    std::uint64_t total_parse_time_ns = 0;

    std::vector<std::uint8_t> buffer(BUFFER_SIZE);

    while (true) {
        std::size_t received = server.receive(buffer.data(), buffer.size());

        if (received == 0) {
            continue;
        }

        ++packets_received;

        if (!mold_parser.parse(buffer.data(), received)) {
            std::cerr << "Invalid MoldUDP64 packet\n";
            continue;
        }

        const std::uint64_t sequence = mold_parser.sequence_number();
        const std::uint16_t message_count = mold_parser.message_count();

        if (mold_parser.is_heartbeat()) {
            std::cout << "Heartbeat: sequence=" << sequence << '\n';
            continue;
        }

        if (mold_parser.is_end_of_session()) {
            std::cout << "End of session\n";
            break;
        }

        if (sequence != expected_sequence) {
            std::cerr << "Sequence gap/error: expected " << expected_sequence
                      << ", received " << sequence << '\n';
        }

        for (std::size_t i = 0; i < message_count; ++i) {
            auto message = mold_parser.message(i);

            if (message.size == 0) {
                continue;
            }

            auto start = std::chrono::steady_clock::now();

            itch_parser.parse_single_message(message.data, message.size);

            auto end = std::chrono::steady_clock::now();

            total_parse_time_ns +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

            ++messages_received;

            if (messages_received % 10000 == 0) { // for debugging only
                std::cout << "Received next 10000 message\n";
            }
        }

        expected_sequence = sequence + message_count;
    }

    std::cout << "\nFeed handler stopped\n";
    std::cout << "Packets received:  " << packets_received << '\n';
    std::cout << "Messages received: " << messages_received << '\n';
    std::cout << "Messages parsed:   " << itch_parser.message_count() << '\n';
    std::cout << "Next sequence:     " << expected_sequence << '\n';

    std::cout << "ITCH parse time:   " << total_parse_time_ns / 1e9 << " s\n";

    if (total_parse_time_ns > 0) {
        std::cout << "ITCH throughput:   "
                  << (messages_received / (total_parse_time_ns / 1e9)) / 1e6
                  << " M msg/s\n";
    }

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