#pragma once

#include <cstddef>
#include <netdb.h>
#include <cstdint>

class UdpClient
{
public:
    UdpClient(const char* hostname, const char* server_port);
    ~UdpClient();
    
    std::size_t send(const std::uint8_t* data, std::size_t size);
    std::size_t receive(std::uint8_t* data, std::size_t size);
    
    UdpClient(const UdpClient&) = delete;
    UdpClient& operator=(const UdpClient&) = delete;

    UdpClient(UdpClient&&) = delete;
    UdpClient& operator=(UdpClient&&) = delete;
    
private:
    int socket_fd_ = -1;
    struct addrinfo hints{};
    struct addrinfo* serv_info_ = nullptr;
};