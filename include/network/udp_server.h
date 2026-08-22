#pragma once

#include <cstddef>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdint>

class UdpServer
{
public:
    UdpServer(const char* sender_port);
    ~UdpServer();

    std::size_t send(const std::uint8_t* data, std::size_t size);
    std::size_t receive(std::uint8_t* data, std::size_t size);

    UdpServer(const UdpServer&) = delete;
    UdpServer& operator=(const UdpServer&) = delete;

    UdpServer(UdpServer&&) = delete;
    UdpServer& operator=(UdpServer&&) = delete;

private:
    int socket_fd_ = -1;
    addrinfo hints{};
    addrinfo* serv_info = nullptr;
    sockaddr_storage their_addr{};
    socklen_t their_addr_len_ = 0;
};