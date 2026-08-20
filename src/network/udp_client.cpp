#include "network/udp_client.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

UdpClient::UdpClient(const char* hostname, const char* server_port)
{
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_DGRAM;  // UDP

    int status = getaddrinfo(hostname, server_port, &hints, &serv_info_);

    if (status != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << '\n';
        return;
    }

    // Try each address returned by getaddrinfo().
    for (addrinfo* p = serv_info_; p != nullptr; p = p->ai_next)
    {
        socket_fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (socket_fd_ == -1)
        {
            std::cerr << "socket: " << std::strerror(errno) << '\n';
            continue;
        }

        success_ = true;
        break;
    }

    if (!success_)
    {
        std::cerr << "failed to create socket\n";

        freeaddrinfo(serv_info_);
        serv_info_ = nullptr;

        return;
    }
}

UdpClient::~UdpClient()
{
    if (socket_fd_ != -1)
        close(socket_fd_);

    if (serv_info_ != nullptr)
        freeaddrinfo(serv_info_);
}

bool UdpClient::success() const
{
    return success_;
}

std::size_t UdpClient::send(const std::uint8_t* data, std::size_t size)
{
    if (!success_)
        return 0;

    ssize_t bytes_sent = sendto(
        socket_fd_, data, size, 0, 
        serv_info_->ai_addr, serv_info_->ai_addrlen
    );

    if (bytes_sent == -1)
    {
        std::cerr << "sendto: " << std::strerror(errno) << '\n';
        return 0;
    }

    return static_cast<std::size_t>(bytes_sent);
}

std::size_t UdpClient::receive(std::uint8_t* data, std::size_t size)
{
    if (!success_)
        return 0;

    ssize_t bytes_received = recvfrom(socket_fd_, data, size, 0, nullptr, nullptr);

    if (bytes_received == -1)
    {
        std::cerr << "recvfrom: " << std::strerror(errno) << '\n';
        return 0;
    }

    return static_cast<std::size_t>(bytes_received);
}

// int main()
// {
//     UdpClient client("localhost", "4950");

//     if (!client.success())
//     {
//         std::cerr << "Failed to create UDP client\n";
//         return 1;
//     }

//     const char* message = "Hello from client!";

//     std::size_t sent = client.send(
//         message,
//         std::strlen(message)
//     );

//     std::cout << "Sent " << sent << " bytes\n";


//     char buffer[1024];

//     std::size_t received = client.receive(buffer, sizeof(buffer));

//     if (received > 0)
//     {
//         std::cout << "Received from server: "
//                 << std::string(buffer, received)
//                 << '\n';

//     }

//     return 0;
// }