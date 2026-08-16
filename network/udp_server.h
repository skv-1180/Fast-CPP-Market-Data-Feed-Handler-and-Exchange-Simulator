#pragma once

#include <cstddef>
#include <sys/socket.h>
#include <netdb.h>

class UdpServer
{
public:
    UdpServer(const char* sender_port);
    ~UdpServer();

    UdpServer(const UdpServer&) = delete;
    UdpServer& operator=(const UdpServer&) = delete;

    bool success() const;

    std::size_t send(const void* data, std::size_t size);
    std::size_t receive(void* data, std::size_t size);

private:
    int socket_fd_ = -1;
    addrinfo hints{};
    addrinfo* serv_info = nullptr;
    sockaddr_storage their_addr{};
    socklen_t their_addr_len_ = 0;
    bool success_ = false;
};