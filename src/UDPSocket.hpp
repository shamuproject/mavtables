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
 *
 *  \warning This class should be treated as pure virtual and should never be
 *      instantiated.
 *
 *  \warning Either \ref send(const std::vector<uint8_t> &data, const IPAddress &) or
 *      send(std::vector<uint8_t>::const_iterator,std::vector<uint8_t>::const_iterator, const IPAddress &)
 *      must be overridden in child classes to avoid infinite recursion.
 *
 *  \warning Either \ref receive(const std::chrono::nanoseconds &) or
 *      receive(std::back_insert_iterator<std::vector<uint8_t>>,const std::chrono::nanoseconds &)
 *      must be overridden in child classes to avoid infinite recursion.
 *
 */
class UDPSocket
{
    public:
        virtual ~UDPSocket();
        virtual void send(
            const std::vector<uint8_t> &data, const IPAddress &address);
        virtual void send(
            std::vector<uint8_t>::const_iterator first,
            std::vector<uint8_t>::const_iterator last,
            const IPAddress &address);
        virtual std::pair<std::vector<uint8_t>, IPAddress> receive(
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());
        virtual IPAddress receive(
            std::back_insert_iterator<std::vector<uint8_t>> it,
            const std::chrono::nanoseconds &timeout =
                std::chrono::nanoseconds::zero());

        friend std::ostream &operator<<(
            std::ostream &os, const UDPSocket &udp_socket);

    protected:
        /** Print the UDP socket to the given output stream.
         *
         *  \param os The output stream to print to.
         *  \returns The output stream.
         */
        virtual std::ostream &print_(std::ostream &os) const;
};


std::ostream &operator<<(std::ostream &os, const UDPSocket &udp_socket);


#endif // UDPSOCKET_HPP_
