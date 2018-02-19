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


#include <chrono>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include "IPAddress.hpp"
#include "UDPSocket.hpp"


// Placed here to avoid weak-vtables error.
// LCOV_EXCL_START
UDPSocket::~UDPSocket()
{
}
// LCOV_EXCL_STOP


/** Send data on the socket to the given address.
 *
 *  \param data The bytes to send.
 *  \param address The address/port to send the bytes to over UDP.
 */
void UDPSocket::send(const std::vector<uint8_t> &data, const IPAddress &address)
{
    send(data.begin(), data.end(), address);
}


/** Receive data on the socket.
 *
 *  \param timeout How long to wait for data to arrive on the socket.
 *  \returns The data read from the socket and the IP address it was sent from.
 */
std::pair<std::vector<uint8_t>, IPAddress> UDPSocket::receive(
    const std::chrono::microseconds &timeout)
{
    std::vector<uint8_t> vec;
    return {vec, receive(std::back_inserter(vec), timeout)};
}
