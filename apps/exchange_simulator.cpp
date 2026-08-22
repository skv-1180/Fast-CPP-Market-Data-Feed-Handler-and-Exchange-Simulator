#include "common/utility.h"
#include "exchange/exchange.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

int run_replay(const char* host, const char* port, const char* filename)
{
    constexpr SeqNo INITIAL_SEQUENCE = 1;
    constexpr std::size_t MAX_PACKET_SIZE = 1400;
    const std::string session = "ITCH50";

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open ITCH file: " + std::string(filename));
    }

    Exchange publisher(host, port, session, INITIAL_SEQUENCE, MAX_PACKET_SIZE);

    std::uint64_t messages_read = 0;
    std::uint64_t previous_timestamp = 0;
    bool first_message = true;

    while (true) {
        char length_buffer[2];

        file.read(length_buffer, sizeof(length_buffer));

        if (file.gcount() == 0)
            break;

        if (file.gcount() != 2) {
            throw std::runtime_error("Truncated message length at EOF");
        }

        const std::uint16_t message_length = md::read_16_bit(length_buffer);

        if (message_length == 0) {
            throw std::runtime_error("Invalid zero-length ITCH message");
        }

        std::vector<std::uint8_t> message(message_length);

        file.read(reinterpret_cast<char*>(message.data()), message_length);

        if (file.gcount() != message_length) {
            break;
            // throw std::runtime_error("Incomplete final ITCH message");
        }

        const std::uint64_t timestamp = md::read_timestamp_6(message.data() + 5);

        if (!first_message) {
            [[maybe_unused]] const std::uint64_t delta = timestamp - previous_timestamp;

            if (messages_read % 100 == 0) {
                constexpr std::uint64_t MAX_SLEEP_NS = 1'000'000;

                std::this_thread::sleep_for(std::chrono::nanoseconds(MAX_SLEEP_NS));
            }

            if (messages_read % 50000 == 0) { // for debugging only
                std::cout << "Message " << messages_read << '\n';
            }
        }

        previous_timestamp = timestamp;
        first_message = false;

        if (!publisher.publish(message.data(), message_length)) {
            throw std::runtime_error("Failed to publish message");
        }

        ++messages_read;
    }

    if (!publisher.flush()) {
        throw std::runtime_error("Failed to flush final packet");
    }

    if (!publisher.send_end_of_session()) {
        throw std::runtime_error("Failed to send end-of-session packet");
    }

    std::cout << "Replay complete\n"
              << "Messages read:  " << messages_read << '\n'
              << "Messages sent:  " << publisher.messages_sent() << '\n'
              << "Packets sent:   " << publisher.packets_sent() << '\n';

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: exchange_simulator " << "<itch_file> <host> <port>\n";
        return 1;
    }

    const char* itch_file = argv[1];
    const char* host = argv[2];
    const char* port = argv[3];

    try {
        return run_replay(host, port, itch_file);
    } catch (const std::exception& e) {
        std::cerr << "Replay failed: " << e.what() << '\n';
        return 1;
    }
}