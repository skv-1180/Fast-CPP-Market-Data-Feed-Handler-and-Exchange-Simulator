#include "udp_server.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>

UdpServer::UdpServer(const char* sender_port)
{
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // ip_v4
    hints.ai_socktype = SOCK_DGRAM; // udp
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    int status = getaddrinfo(nullptr, sender_port, &hints, &serv_info);
    if (status != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return;  
    }

    addrinfo *p;

    // loop through all the results and bind to the first we can
    for(p = serv_info; p != nullptr; p = p->ai_next) 
    {
        socket_fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (socket_fd_ == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(socket_fd_, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd_); 
            perror("listener: bind");
            continue;
        }
        break;
    }

    if (p == nullptr) {
        std::cerr << "server: failed to bind socket" << std::endl;
        return;
    }

    success_ = true;
    std::cout << "Server started successfully!" << std::endl;
}

UdpServer::~UdpServer()
{
    if (socket_fd_ != -1)
        close(socket_fd_);

    if (serv_info != nullptr)
        freeaddrinfo(serv_info);
    std::cout << "Server closed!" << std::endl;
}

bool UdpServer::success() const
{
    return success_;
}

std::size_t UdpServer::receive(void* data, std::size_t size)
{
    if (!success_)
        return 0;

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

std::size_t UdpServer::send(const void* data, std::size_t size)
{
    if (!success_)
        return 0;

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
//     UdpServer server("4950");
//     if (!server.success())
//     {
//         std::cerr << "Failed to create UDP server\n";
//         return 1;
//     }
    
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
