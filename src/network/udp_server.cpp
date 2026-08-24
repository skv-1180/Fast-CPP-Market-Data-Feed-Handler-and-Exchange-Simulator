#include "network/udp_server.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

#include <stdexcept>
#include <system_error>
#include <sys/types.h>
#include <netdb.h>

UdpServer::UdpServer(const char* sender_port)
{
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_flags = AI_PASSIVE;   

    int status = getaddrinfo(nullptr, sender_port, &hints, &serv_info);
    if (status != 0)
    {
        throw std::runtime_error(std::string("getaddrinfo failed: ") + gai_strerror(status));
    }

    addrinfo* p = nullptr;
    for (p = serv_info; p != nullptr; p = p->ai_next)
    {
        socket_fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd_ == -1)
        {
            continue;
        }

        // Increase UDP receive buffer
        int receive_buffer = 16 * 1024 * 1024; 
        if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &receive_buffer, sizeof(receive_buffer)) == -1)
        {
            close(socket_fd_);
            socket_fd_ = -1;
            continue;
        }

        // Check the actual buffer size assigned by the kernel
        // int actual_buffer = 0;
        // socklen_t actual_buffer_len = sizeof(actual_buffer);
        // if (getsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &actual_buffer, &actual_buffer_len) == 0)
        // {
        //     std::cout << "UDP receive buffer: " << actual_buffer << " bytes\n";
        // }

        if (bind(socket_fd_, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd_);
            socket_fd_ = -1;
            continue;
        }

        break; 
    }

    if (p == nullptr)
    {
        int saved_errno = errno;
        freeaddrinfo(serv_info);
        serv_info = nullptr;

        throw std::system_error(saved_errno, std::generic_category(), "Failed to bind UDP server socket");
    }
}

UdpServer::~UdpServer()
{
    if (socket_fd_ != -1)
        close(socket_fd_);

    if (serv_info != nullptr)
        freeaddrinfo(serv_info);
}

std::size_t UdpServer::receive(std::uint8_t* data, std::size_t size)
{
    their_addr_len_ = sizeof(their_addr);

    ssize_t bytes_received = recvfrom(
        socket_fd_, data, size, 0, 
        reinterpret_cast<sockaddr*>(&their_addr), &their_addr_len_
    );

    if (bytes_received == -1)
    {
        std::cerr << "recvfrom: " << std::strerror(errno) << '\n';
        return 0;
    }

    return static_cast<std::size_t>(bytes_received);
}

std::size_t UdpServer::send(const std::uint8_t* data, std::size_t size)
{
    if (their_addr_len_ == 0)
    {
        std::cerr << "send: no client address available\n";
        return 0;
    }

    ssize_t bytes_sent = sendto( 
        socket_fd_, data, size, 0, 
        reinterpret_cast<const struct sockaddr*>(&their_addr), their_addr_len_
    );

    if (bytes_sent == -1)
    {
        std::cerr << "sendto: " << std::strerror(errno) << '\n';
        return 0;
    }

    return static_cast<std::size_t>(bytes_sent);
}

// int main(){
//     UdpServer server("18000");
    
//     char buffer[1024];

//     std::size_t received = server.receive(buffer, sizeof(buffer));

//     if (received > 0)
//     {
//         std::cout << "Received: "
//                 << std::string(buffer, received)
//                 << '\n';

//         const char* response = "Hello from server!";

//         server.send(response, std::strlen(response));
//     }
// }
