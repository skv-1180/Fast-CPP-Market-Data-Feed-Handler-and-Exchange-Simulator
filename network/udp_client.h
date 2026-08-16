#pragma once

#include <cstddef>
#include <netdb.h>

class UdpClient
{
public:
    UdpClient(const char* hostname, const char* server_port);
    ~UdpClient();

    bool success() const;
    
    std::size_t send(const void* data, std::size_t size);
    std::size_t receive(void* data, std::size_t size);
    
    UdpClient(const UdpClient&) = delete;
    UdpClient& operator=(const UdpClient&) = delete;
    
private:
    int socket_fd_ = -1;
    struct addrinfo hints{};
    struct addrinfo* serv_info_ = nullptr;
    bool success_ = false;
};