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


#ifndef UDPSOCKET_HPP_
#define UDPSOCKET_HPP_


#include <chrono>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include "IPAddress.hpp"


/** A UDP socket, listening on a port/address combination.
 */
class UDPSocket
{
    public:
        /** Construct a UDP socket.
         *
         *  \param port The port number to listen on.
         *  \param address The address to listen on.  The default is to listen
         *      on any address.
         */
        UDPSocket(unsigned int port, std::optional<IPAddress> address = {});
        virtual ~UDPSocket();
        void send(const std::vector<uint8_t> &data, const IPAddress &address);
        /** Send data on the socket to the given address.
         *
         *  \param first Iterator to first byte in range to send.
         *  \param last Iterator to one past the last byte to send.
         *  \param address IP address (with port number) to send the bytes to.
         */
        virtual void send(
            std::vector<uint8_t>::const_iterator first,
            std::vector<uint8_t>::const_iterator last,
            const IPAddress &address) = 0;
        std::pair<std::vector<uint8_t>, IPAddress> receive(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds(100000));
        /** Receive data on the socket.
         *
         *  \param it A back insert iterator to read bytes into.
         *  \param timeout How long to wait for data to arrive on the socket.
         *  \returns The IP address the data was sent from, this is where a
         *      reply should be sent to.
         */
        virtual IPAddress receive(
            std::back_insert_iterator<std::vector<uint8_t>> it,
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds(100000)) = 0;
};


#endif // UDPSOCKET_HPP_
