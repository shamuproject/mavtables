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
#include <iterator>
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


/** Send data on the socket to the given address.
 *
 *  \param first Iterator to first byte in range to send.
 *  \param last Iterator to one past the last byte to send.
 *  \param address IP address (with port number) to send the bytes to.
 */
void UDPSocket::send(
    std::vector<uint8_t>::const_iterator first,
    std::vector<uint8_t>::const_iterator last,
    const IPAddress &address)
{
    std::vector<uint8_t> vec;
    std::copy(first, last, std::back_inserter(vec));
    send(vec, address);
}


/** Receive data on the socket.
 *
 *  \note The \p timeout is not guaranteed to be up to nanosecond
 *      precision, the actual precision is up to the operating system's
 *      implementation but is guaranteed to be have at least millisecond
 *      precision.
 *
 *  \param timeout How long to wait for data to arrive on the socket.  The
 *      default is to not wait.
 *  \returns The data read from the socket and the IP address it was sent from.
 */
std::pair<std::vector<uint8_t>, IPAddress> UDPSocket::receive(
    const std::chrono::nanoseconds &timeout)
{
    std::vector<uint8_t> vec;
    return {vec, receive(std::back_inserter(vec), timeout)};
}


/** Receive data on the socket.
 *
 *  \note The \p timeout is not guaranteed to be up to nanosecond
 *      precision, the actual precision is up to the operating system's
 *      implementation but is guaranteed to be have at least millisecond
 *      precision.
 *
 *  \param it A back insert iterator to read bytes into.
 *  \param timeout How long to wait for data to arrive on the socket.
 *      The default is to not wait.
 *  \returns The IP address the data was sent from, this is where a
 *      reply should be sent to.
 */
IPAddress UDPSocket::receive(
    std::back_insert_iterator<std::vector<uint8_t>> it,
    const std::chrono::nanoseconds &timeout)
{
    auto [vec, address] = receive(timeout);
    std::copy(vec.begin(), vec.end(), it);
    return address;
}


std::ostream &UDPSocket::print_(std::ostream &os) const
{
    os << "unknown UDP socket";
    return os;
}


/** Print the given UDP socket to the given output stream.
 *
 *  An example:
 *  ```
 *  serial {
 *      device /dev/ttyUSB0;
 *      baudrate 115200;
 *      flow_control yes;
 *  }
 *  ```
 
 *  \relates SerialPort
 *  \param os The output stream to print to.
 *  \param udp_socket The UDP socket (or any child of UDPSocket) to print.
 *  \return The output stream.
 */
std::ostream &operator<<(std::ostream &os, const UDPSocket &udp_socket)
{
    return udp_socket.print_(os);
}
