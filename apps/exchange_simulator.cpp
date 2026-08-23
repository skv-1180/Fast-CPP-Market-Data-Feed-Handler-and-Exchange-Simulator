#include "common/utility.h"
#include "moldudp64/moldudp64_protocol.h"
#include "exchange/exchange.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <bit>

int run_replay(const char* host, const char* port, const char* filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open ITCH file: " + std::string(filename));
    }
    
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> file_buffer(file_size);
    if (!file.read(reinterpret_cast<char*>(file_buffer.data()), file_size)) {
        throw std::runtime_error("Failed to read file into memory");
    }

    Exchange publisher(host, port, "ITCH50", INITIAL_SEQUENCE, MAX_PACKET_SIZE);

    std::uint64_t messages_read = 0;
    
    const std::uint8_t* ptr = file_buffer.data();
    const std::uint8_t* end = ptr + file_size;

    auto start_time = std::chrono::steady_clock::now();

    while (ptr + 2 <= end) 
    {
        const std::uint16_t message_length = 
            std::byteswap(*reinterpret_cast<const std::uint16_t*>(ptr));
        ptr += 2;

        if (message_length == 0 || ptr + message_length > end) [[unlikely]] 
        {
            break; 
        }

        if (messages_read > 0) [[likely]]
        {
            if (messages_read % 5000 == 0) [[unlikely]] 
            {
                constexpr std::chrono::nanoseconds MAX_SLEEP_NS{1'000'000};
                std::this_thread::sleep_for(MAX_SLEEP_NS);
            }

            if (messages_read % 50000 == 0) [[unlikely]] 
            {
                std::cout << "Message " << messages_read << '\n';
            }
        }

        if (!publisher.publish(ptr, message_length)) [[unlikely]] 
        {
            throw std::runtime_error("Failed to publish message");
        }

        ptr += message_length;
        ++messages_read;
    }

    if (!publisher.flush()) {
        throw std::runtime_error("Failed to flush final packet");
    }

    if (!publisher.send_end_of_session()) {
        throw std::runtime_error("Failed to send end-of-session packet");
    }

    auto end_time = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(end_time - start_time).count();

    std::cout << "Replay complete\n"
              << "Messages read:  " << messages_read << '\n'
              << "Messages sent:  " << publisher.messages_sent() << '\n'
              << "Packets sent:   " << publisher.packets_sent() << '\n'
              << "Elapsed time:   " << elapsed_sec << " seconds\n"
              << "Paced Rate:     " << (publisher.messages_sent() / elapsed_sec) << " msg/s\n";

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