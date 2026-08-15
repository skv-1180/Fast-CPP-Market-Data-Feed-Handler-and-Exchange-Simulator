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
        std::cerr << "Cannot open file\n";
        return 1;
    }

    Market market;
    Itch50Parser<Market> parser(market);

    constexpr size_t BUFFER_SIZE = 1 << 20;  // 1 MB
    std::vector<char> buffer(BUFFER_SIZE * 2);
    size_t leftover = 0;

    while (file)
    {
        file.read(buffer.data() + leftover, BUFFER_SIZE);
        size_t bytes = leftover + file.gcount();

        size_t consumed = parser.parse_multiple_message(buffer.data(), bytes);

        leftover = bytes - consumed;

        std::memmove(buffer.data(),
                     buffer.data() + consumed,
                     leftover);
    }

    std::cout << "Replay complete\n";
}