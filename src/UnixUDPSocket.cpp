// MAVLink router and firewall.
// Copyright (C) 2018  Michael R. Shannon <mrshannon.aerospace@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <errno.h>

#include "IPAddress.hpp"
#include "UnixSyscalls.hpp"
#include "UnixUDPSocket.hpp"


/** Construct a UDP socket.
 *
 *  \param port The port number to listen on.
 *  \param address The address to listen on (the port portion of the address is
 *      ignored).  The default is to listen on any address.
 *  \param syscalls The object to use for Unix system calls.  It is default
 *      constructed to the production implementation.
 */
UnixUDPSocket::UnixUDPSocket(
    unsigned int port, std::optional<IPAddress> address,
    std::unique_ptr<UnixSyscalls> syscalls)
    : port_(port), address_(std::move(address)),
      syscalls_(std::move(syscalls)), socket_(-1)
{
}


/** The socket destructor.
 *
 *  This closes the underlying file descriptor.
 */
UnixUDPSocket::~UnixUDPSocket()
{
    syscalls_->close(socket_);
}


/** \copydoc UDPSocket::send(std::vector<uint8_t>::const_iterator,std::vector<uint8_t>::const_iterator,const IPAddress &)
 */
void UnixUDPSocket::send(
    std::vector<uint8_t>::const_iterator first,
    std::vector<uint8_t>::const_iterator last,
    const IPAddress &address)
{
    (void)first;
    (void)last;
    (void)address;
}


/** \copydoc UDPSocket::receive(const std::chrono::nanoseconds &)
 *
 *  \note The timeout precision of this implementation is 1 millisecond.
 */
std::pair<std::vector<uint8_t>, IPAddress> UnixUDPSocket::receive(
    const std::chrono::nanoseconds &timeout)
{
    std::chrono::milliseconds timeout_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
    struct pollfd fd = {socket_, POLLIN, 0};
    auto result = syscalls_->poll(&fd, 1, static_cast<int>(timeout_ms.count()));

    // Poll error
    if (result < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
    // Success
    else if (result > 0)
    {
        // Socket error
        if (fd.revents & POLLERR)
        {
            close(socket_);
            create_socket_();
            return {std::vector<uint8_t>(), IPAddress(0)};
        }
        // Datagram available for reading.
        else if (fd.revents & POLLIN)
        {
            return receive_();
        }
    }

    // Timed out
    return {std::vector<uint8_t>(), IPAddress(0)};
}


/** Create socket given port_ and address_ member variables.
 */
void UnixUDPSocket::create_socket_()
{
    socket_ = -1;

    // Create socket.
    if ((socket_ = syscalls_->socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    // Bind socket to port (and optionally an IP address).
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));

    if (address_)
    {
        addr.sin_addr.s_addr =
            htonl(static_cast<uint32_t>(address_.value().address()));
    }
    else
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    memset(addr.sin_zero, '\0', sizeof addr.sin_zero);

    if ((syscalls_->bind(socket_, reinterpret_cast<struct sockaddr *>(&addr),
                         sizeof(addr))) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
}


/** Read data from socket.
 *
 *  \note There must be data to read, otherwise calling this method is
 *      undefined.
 *
 *  \returns The data read from the socket and the IP address it was sent from.
 */
std::pair<std::vector<uint8_t>, IPAddress> UnixUDPSocket::receive_()
{
    // Get size of buffer required and create it.
    int packet_size;

    if ((syscalls_->ioctl(socket_, FIONREAD, &packet_size)) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    std::vector<uint8_t> buffer;
    buffer.resize(static_cast<size_t>(packet_size));
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    auto size = syscalls_->recvfrom(
               socket_, buffer.data(), buffer.size(), 0,
               reinterpret_cast<struct sockaddr *>(&addr), &addrlen);

    if (size < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
    else if (size > 0)
    {
        if (addrlen <= sizeof(addr) && addr.sin_family == AF_INET)
        {
            auto ip =
                IPAddress(ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port));
            return {buffer, ip};
        }
    }

    return {std::vector<uint8_t>(), IPAddress(0)};
}

