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
#include <memory>
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
    : port_(port), address_(std::move(address)), syscalls_(std::move(syscalls)),
      socket_(-1)
{
    create_socket_();
}


/** The socket destructor.
 *
 *  This closes the underlying file descriptor.
 */
// LCOV_EXCL_START
UnixUDPSocket::~UnixUDPSocket() { syscalls_->close(socket_); }
// LCOV_EXCL_STOP


/** \copydoc UDPSocket::send(const std::vector<uint8_t> &, const IPAddress &)
 */
void UnixUDPSocket::send(
    const std::vector<uint8_t> &data, const IPAddress &address)
{
    // Destination address structure.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(address.port()));
    addr.sin_addr.s_addr = htonl(static_cast<uint32_t>(address.address()));
    std::memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
    // Send the packet.
    auto err = syscalls_->sendto(
        socket_, data.data(), data.size(), 0,
        reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));

    if (err < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
}


/** \copydoc UDPSocket::receive(const std::chrono::nanoseconds &)
 *
 *  \note The timeout precision of this implementation is 1 millisecond.
 */
std::pair<std::vector<uint8_t>, IPAddress>
UnixUDPSocket::receive(const std::chrono::nanoseconds &timeout)
{
    std::chrono::milliseconds timeout_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
    struct pollfd fds = {socket_, POLLIN, 0};
    auto result =
        syscalls_->poll(&fds, 1, static_cast<int>(timeout_ms.count()));

    // Poll error
    if (result < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
    // Success
    else if (result > 0)
    {
        // Socket error
        if (fds.revents & POLLERR)
        {
            syscalls_->close(socket_);
            create_socket_();
            return {std::vector<uint8_t>(), IPAddress(0)};
        }
        // Datagram available for reading.
        else if (fds.revents & POLLIN)
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

    std::memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if ((syscalls_->bind(
            socket_, reinterpret_cast<struct sockaddr *>(&addr),
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
    // Get needed buffer size.
    int packet_size;

    if ((syscalls_->ioctl(socket_, FIONREAD, &packet_size)) < 0)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    // Read datagram.
    std::vector<uint8_t> buffer;
    buffer.resize(static_cast<size_t>(packet_size));
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    auto size = syscalls_->recvfrom(
        socket_, buffer.data(), buffer.size(), 0,
        reinterpret_cast<struct sockaddr *>(&addr), &addrlen);

    // Handle errors and extract IP address.
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

    // Failed to read datagram.
    return {std::vector<uint8_t>(), IPAddress(0)};
}
